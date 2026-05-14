#include "dlms/apdu/action.hpp"

#include "dlms/apdu/apdu_reader.hpp"
#include "dlms/apdu/axdr.hpp"

namespace dlms {
namespace apdu {

namespace {

constexpr std::uint8_t kActionRequestTag = 0xC3;
constexpr std::uint8_t kActionResponseTag = 0xC7;
constexpr std::uint8_t kNormalChoice = 0x01;
constexpr std::uint8_t kNextPblockChoice = 0x02;
constexpr std::uint8_t kWithListChoice = 0x03;
constexpr std::uint8_t kWithFirstPblockChoice = 0x04;
constexpr std::uint8_t kWithListAndFirstPblockChoice = 0x05;
constexpr std::uint8_t kWithPblockChoice = 0x06;

ApduStatus RequireNoTrailingBytes(ApduReader& reader)
{
  return reader.Empty() ? ApduStatus::Ok : ApduStatus::InvalidLength;
}

ApduStatus ReadRawData(ApduReader& reader, ByteView& output)
{
  AxdrOctetString value = {};
  ApduStatus status = ReadAxdrOctetString(reader, value);
  if (status != ApduStatus::Ok) {
    return status;
  }
  output.data = value.data;
  output.size = value.size;
  return ApduStatus::Ok;
}

ApduStatus WriteRawData(ApduWriter& writer, ByteView input)
{
  return WriteAxdrOctetString(writer, input.data, input.size);
}

ApduStatus DecodeDataBlockSA(ApduReader& reader, DataBlockSA& output)
{
  ApduStatus status = ReadAxdrBoolean(reader, output.lastBlock);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = reader.ReadU32(output.blockNumber);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return ReadRawData(reader, output.rawData);
}

ApduStatus EncodeDataBlockSA(const DataBlockSA& input, ApduWriter& writer)
{
  ApduStatus status = WriteAxdrBoolean(writer, input.lastBlock);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = writer.WriteU32(input.blockNumber);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return WriteRawData(writer, input.rawData);
}

ApduStatus DecodeDataBlockG(ApduReader& reader, DataBlockG& output)
{
  ApduStatus status = ReadAxdrBoolean(reader, output.lastBlock);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = reader.ReadU32(output.blockNumber);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return ReadRawData(reader, output.rawData);
}

ApduStatus EncodeDataBlockG(const DataBlockG& input, ApduWriter& writer)
{
  ApduStatus status = WriteAxdrBoolean(writer, input.lastBlock);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = writer.WriteU32(input.blockNumber);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return WriteRawData(writer, input.rawData);
}

ApduStatus DecodeOptionalData(
  ApduReader& reader,
  std::size_t maximumDataDepth,
  bool& present,
  DlmsData& data)
{
  std::uint8_t flag = 0;
  ApduStatus status = reader.ReadU8(flag);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (flag > 1U) {
    return ApduStatus::InvalidChoice;
  }
  present = flag != 0U;
  if (!present) {
    return ApduStatus::Ok;
  }
  return DecodeDlmsDataFromReader(reader, maximumDataDepth, data);
}

ApduStatus DecodeOptionalActionReturnParameter(
  ApduReader& reader,
  std::size_t maximumDataDepth,
  bool& present,
  DlmsData& data)
{
  std::uint8_t flag = 0;
  ApduStatus status = reader.ReadU8(flag);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (flag > 1U) {
    return ApduStatus::InvalidChoice;
  }
  present = flag != 0U;
  if (!present) {
    return ApduStatus::Ok;
  }

  std::uint8_t choice = 0;
  status = reader.ReadU8(choice);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (choice != 0U) {
    return ApduStatus::UnsupportedXdlmsService;
  }
  return DecodeDlmsDataFromReader(reader, maximumDataDepth, data);
}

ApduStatus EncodeOptionalData(
  bool present,
  const DlmsData& data,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(present ? 0x01 : 0x00);
  if (status != ApduStatus::Ok || !present) {
    return status;
  }
  return EncodeDlmsData(data, writer);
}

ApduStatus EncodeOptionalActionReturnParameter(
  bool present,
  const DlmsData& data,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(present ? 0x01 : 0x00);
  if (status != ApduStatus::Ok || !present) {
    return status;
  }
  status = writer.WriteU8(0x00);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return EncodeDlmsData(data, writer);
}

ApduStatus DecodeMethodWithParameter(
  ApduReader& reader,
  std::size_t maximumDataDepth,
  CosemMethodDescriptorWithParameter& output)
{
  ApduStatus status = DecodeCosemMethodDescriptor(reader, output.descriptor);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return DecodeOptionalData(
    reader,
    maximumDataDepth,
    output.hasInvocationParameter,
    output.invocationParameter);
}

ApduStatus EncodeMethodWithParameter(
  const CosemMethodDescriptorWithParameter& input,
  ApduWriter& writer)
{
  ApduStatus status = EncodeCosemMethodDescriptor(input.descriptor, writer);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return EncodeOptionalData(
    input.hasInvocationParameter,
    input.invocationParameter,
    writer);
}

ApduStatus DecodeResponseItem(
  ApduReader& reader,
  std::size_t maximumDataDepth,
  ActionResponseItem& output)
{
  ApduStatus status = reader.ReadU8(output.result);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return DecodeOptionalActionReturnParameter(
    reader,
    maximumDataDepth,
    output.hasReturnParameter,
    output.returnParameter);
}

ApduStatus EncodeResponseItem(
  const ActionResponseItem& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(input.result);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return EncodeOptionalActionReturnParameter(
    input.hasReturnParameter,
    input.returnParameter,
    writer);
}

} // namespace

ApduStatus DecodeActionRequestNormal(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  ActionRequestNormal& output)
{
  if (input == nullptr && inputSize != 0) {
    return ApduStatus::InvalidArgument;
  }

  output = {};
  ApduReader reader(input, inputSize);

  std::uint8_t value = 0;
  ApduStatus status = reader.ReadU8(value);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (value != kActionRequestTag) {
    return ApduStatus::InvalidTag;
  }

  status = reader.ReadU8(value);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (value != kNormalChoice) {
    return ApduStatus::UnsupportedXdlmsService;
  }

  status = reader.ReadU8(output.invokeIdAndPriority);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = DecodeCosemMethodDescriptor(reader, output.descriptor);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = DecodeOptionalData(
    reader,
    maximumDataDepth,
    output.hasInvocationParameter,
    output.invocationParameter);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return RequireNoTrailingBytes(reader);
}

ApduStatus EncodeActionRequestNormal(
  const ActionRequestNormal& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(kActionRequestTag);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = writer.WriteU8(kNormalChoice);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = writer.WriteU8(input.invokeIdAndPriority);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = EncodeCosemMethodDescriptor(input.descriptor, writer);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return EncodeOptionalData(
    input.hasInvocationParameter,
    input.invocationParameter,
    writer);
}

ApduStatus DecodeActionResponseNormal(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  ActionResponseNormal& output)
{
  if (input == nullptr && inputSize != 0) {
    return ApduStatus::InvalidArgument;
  }

  output = {};
  ApduReader reader(input, inputSize);

  std::uint8_t value = 0;
  ApduStatus status = reader.ReadU8(value);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (value != kActionResponseTag) {
    return ApduStatus::InvalidTag;
  }

  status = reader.ReadU8(value);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (value != kNormalChoice) {
    return ApduStatus::UnsupportedXdlmsService;
  }

  status = reader.ReadU8(output.invokeIdAndPriority);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = reader.ReadU8(output.result);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = DecodeOptionalActionReturnParameter(
    reader,
    maximumDataDepth,
    output.hasReturnParameter,
    output.returnParameter);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return RequireNoTrailingBytes(reader);
}

ApduStatus EncodeActionResponseNormal(
  const ActionResponseNormal& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(kActionResponseTag);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = writer.WriteU8(kNormalChoice);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = writer.WriteU8(input.invokeIdAndPriority);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = writer.WriteU8(input.result);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return EncodeOptionalActionReturnParameter(
    input.hasReturnParameter,
    input.returnParameter,
    writer);
}

ApduStatus DecodeActionRequest(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  ActionRequest& output)
{
  if (input == nullptr && inputSize != 0) {
    return ApduStatus::InvalidArgument;
  }

  output = {};
  ApduReader reader(input, inputSize);
  std::uint8_t tag = 0;
  ApduStatus status = reader.ReadU8(tag);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (tag != kActionRequestTag) {
    return ApduStatus::InvalidTag;
  }
  std::uint8_t choice = 0;
  status = reader.ReadU8(choice);
  if (status != ApduStatus::Ok) {
    return status;
  }
  output.choice = static_cast<ActionRequestChoice>(choice);
  status = reader.ReadU8(output.invokeIdAndPriority);
  if (status != ApduStatus::Ok) {
    return status;
  }

  switch (choice) {
    case kNormalChoice:
      status = DecodeMethodWithParameter(reader, maximumDataDepth, output.normal);
      break;

    case kNextPblockChoice:
      status = reader.ReadU32(output.blockNumber);
      break;

    case kWithListChoice:
      {
        std::size_t count = 0;
        status = ReadAxdrLength(reader, count);
        if (status != ApduStatus::Ok) {
          return status;
        }
        output.list.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
          CosemMethodDescriptorWithParameter item = {};
          status = DecodeMethodWithParameter(reader, maximumDataDepth, item);
          if (status != ApduStatus::Ok) {
            return status;
          }
          output.list.push_back(item);
        }
      }
      break;

    case kWithFirstPblockChoice:
      status = DecodeCosemMethodDescriptor(reader, output.normal.descriptor);
      if (status != ApduStatus::Ok) {
        return status;
      }
      status = DecodeDataBlockSA(reader, output.dataBlock);
      break;

    case kWithListAndFirstPblockChoice:
      {
        std::size_t count = 0;
        status = ReadAxdrLength(reader, count);
        if (status != ApduStatus::Ok) {
          return status;
        }
        output.descriptorList.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
          CosemMethodDescriptor item = {};
          status = DecodeCosemMethodDescriptor(reader, item);
          if (status != ApduStatus::Ok) {
            return status;
          }
          output.descriptorList.push_back(item);
        }
        status = DecodeDataBlockSA(reader, output.dataBlock);
      }
      break;

    case kWithPblockChoice:
      status = DecodeDataBlockSA(reader, output.dataBlock);
      break;

    default:
      return ApduStatus::UnsupportedXdlmsService;
  }
  if (status != ApduStatus::Ok) {
    return status;
  }
  return RequireNoTrailingBytes(reader);
}

ApduStatus EncodeActionRequest(
  const ActionRequest& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(kActionRequestTag);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = writer.WriteU8(static_cast<std::uint8_t>(input.choice));
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = writer.WriteU8(input.invokeIdAndPriority);
  if (status != ApduStatus::Ok) {
    return status;
  }

  switch (input.choice) {
    case ActionRequestChoice::Normal:
      return EncodeMethodWithParameter(input.normal, writer);

    case ActionRequestChoice::NextPblock:
      return writer.WriteU32(input.blockNumber);

    case ActionRequestChoice::WithList:
      status = WriteAxdrLength(writer, input.list.size());
      if (status != ApduStatus::Ok) {
        return status;
      }
      for (std::size_t i = 0; i < input.list.size(); ++i) {
        status = EncodeMethodWithParameter(input.list[i], writer);
        if (status != ApduStatus::Ok) {
          return status;
        }
      }
      return ApduStatus::Ok;

    case ActionRequestChoice::WithFirstPblock:
      status = EncodeCosemMethodDescriptor(input.normal.descriptor, writer);
      if (status != ApduStatus::Ok) {
        return status;
      }
      return EncodeDataBlockSA(input.dataBlock, writer);

    case ActionRequestChoice::WithListAndFirstPblock:
      status = WriteAxdrLength(writer, input.descriptorList.size());
      if (status != ApduStatus::Ok) {
        return status;
      }
      for (std::size_t i = 0; i < input.descriptorList.size(); ++i) {
        status = EncodeCosemMethodDescriptor(input.descriptorList[i], writer);
        if (status != ApduStatus::Ok) {
          return status;
        }
      }
      return EncodeDataBlockSA(input.dataBlock, writer);

    case ActionRequestChoice::WithPblock:
      return EncodeDataBlockSA(input.dataBlock, writer);
  }
  return ApduStatus::InvalidChoice;
}

ApduStatus DecodeActionResponse(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  ActionResponse& output)
{
  if (input == nullptr && inputSize != 0) {
    return ApduStatus::InvalidArgument;
  }

  output = {};
  ApduReader reader(input, inputSize);
  std::uint8_t tag = 0;
  ApduStatus status = reader.ReadU8(tag);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (tag != kActionResponseTag) {
    return ApduStatus::InvalidTag;
  }
  std::uint8_t choice = 0;
  status = reader.ReadU8(choice);
  if (status != ApduStatus::Ok) {
    return status;
  }
  output.choice = static_cast<ActionResponseChoice>(choice);
  status = reader.ReadU8(output.invokeIdAndPriority);
  if (status != ApduStatus::Ok) {
    return status;
  }

  switch (choice) {
    case kNormalChoice:
      status = DecodeResponseItem(reader, maximumDataDepth, output.normal);
      break;

    case kNextPblockChoice:
      status = DecodeDataBlockG(reader, output.dataBlock);
      break;

    case kWithListChoice:
      {
        std::size_t count = 0;
        status = ReadAxdrLength(reader, count);
        if (status != ApduStatus::Ok) {
          return status;
        }
        output.list.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
          ActionResponseItem item = {};
          status = DecodeResponseItem(reader, maximumDataDepth, item);
          if (status != ApduStatus::Ok) {
            return status;
          }
          output.list.push_back(item);
        }
      }
      break;

    case kWithFirstPblockChoice:
      status = reader.ReadU32(output.blockNumber);
      break;

    default:
      return ApduStatus::UnsupportedXdlmsService;
  }
  if (status != ApduStatus::Ok) {
    return status;
  }
  return RequireNoTrailingBytes(reader);
}

ApduStatus EncodeActionResponse(
  const ActionResponse& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(kActionResponseTag);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = writer.WriteU8(static_cast<std::uint8_t>(input.choice));
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = writer.WriteU8(input.invokeIdAndPriority);
  if (status != ApduStatus::Ok) {
    return status;
  }

  switch (input.choice) {
    case ActionResponseChoice::Normal:
      return EncodeResponseItem(input.normal, writer);

    case ActionResponseChoice::WithPblock:
      return EncodeDataBlockG(input.dataBlock, writer);

    case ActionResponseChoice::WithList:
      status = WriteAxdrLength(writer, input.list.size());
      if (status != ApduStatus::Ok) {
        return status;
      }
      for (std::size_t i = 0; i < input.list.size(); ++i) {
        status = EncodeResponseItem(input.list[i], writer);
        if (status != ApduStatus::Ok) {
          return status;
        }
      }
      return ApduStatus::Ok;

    case ActionResponseChoice::NextPblock:
      return writer.WriteU32(input.blockNumber);
  }
  return ApduStatus::InvalidChoice;
}

} // namespace apdu
} // namespace dlms
