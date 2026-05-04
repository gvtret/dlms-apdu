#include "dlms/apdu/set.hpp"

#include "dlms/apdu/apdu_reader.hpp"
#include "dlms/apdu/axdr.hpp"

namespace dlms {
namespace apdu {

namespace {

constexpr std::uint8_t kSetRequestTag = 0xC1;
constexpr std::uint8_t kSetResponseTag = 0xC5;
constexpr std::uint8_t kNormalChoice = 0x01;
constexpr std::uint8_t kWithFirstDataBlockChoice = 0x02;
constexpr std::uint8_t kWithDataBlockChoice = 0x03;
constexpr std::uint8_t kWithListChoice = 0x04;
constexpr std::uint8_t kWithListAndFirstDataBlockChoice = 0x05;

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

ApduStatus DecodeSelection(
  ApduReader& reader,
  std::size_t maximumDataDepth,
  bool& present,
  SelectiveAccessDescriptor& output)
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
  status = reader.ReadU8(output.selector);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return DecodeDlmsDataFromReader(reader, maximumDataDepth, output.parameters);
}

ApduStatus EncodeSelection(
  bool present,
  const SelectiveAccessDescriptor& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(present ? 0x01 : 0x00);
  if (status != ApduStatus::Ok || !present) {
    return status;
  }
  status = writer.WriteU8(input.selector);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return EncodeDlmsData(input.parameters, writer);
}

ApduStatus DecodeDescriptorWithSelection(
  ApduReader& reader,
  std::size_t maximumDataDepth,
  CosemAttributeDescriptorWithSelection& output)
{
  ApduStatus status = DecodeCosemAttributeDescriptor(reader, output.descriptor);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return DecodeSelection(reader, maximumDataDepth, output.hasSelection, output.selection);
}

ApduStatus EncodeDescriptorWithSelection(
  const CosemAttributeDescriptorWithSelection& input,
  ApduWriter& writer)
{
  ApduStatus status = EncodeCosemAttributeDescriptor(input.descriptor, writer);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return EncodeSelection(input.hasSelection, input.selection, writer);
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

ApduStatus DecodeResultList(ApduReader& reader, std::vector<std::uint8_t>& output)
{
  std::size_t count = 0;
  ApduStatus status = ReadAxdrLength(reader, count);
  if (status != ApduStatus::Ok) {
    return status;
  }
  output.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    std::uint8_t result = 0;
    status = reader.ReadU8(result);
    if (status != ApduStatus::Ok) {
      return status;
    }
    output.push_back(result);
  }
  return ApduStatus::Ok;
}

ApduStatus EncodeResultList(
  const std::vector<std::uint8_t>& input,
  ApduWriter& writer)
{
  ApduStatus status = WriteAxdrLength(writer, input.size());
  if (status != ApduStatus::Ok) {
    return status;
  }
  for (std::size_t i = 0; i < input.size(); ++i) {
    status = writer.WriteU8(input[i]);
    if (status != ApduStatus::Ok) {
      return status;
    }
  }
  return ApduStatus::Ok;
}

} // namespace

ApduStatus DecodeSetRequestNormal(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  SetRequestNormal& output)
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
  if (value != kSetRequestTag) {
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

  status = DecodeCosemAttributeDescriptor(reader, output.descriptor);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = DecodeSelection(
    reader,
    maximumDataDepth,
    output.hasSelectiveAccess,
    output.selectiveAccess);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = DecodeDlmsDataFromReader(reader, maximumDataDepth, output.data);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return RequireNoTrailingBytes(reader);
}

ApduStatus EncodeSetRequestNormal(
  const SetRequestNormal& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(kSetRequestTag);
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
  status = EncodeCosemAttributeDescriptor(input.descriptor, writer);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = EncodeSelection(input.hasSelectiveAccess, input.selectiveAccess, writer);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return EncodeDlmsData(input.data, writer);
}

ApduStatus DecodeSetResponseNormal(
  const std::uint8_t* input,
  std::size_t inputSize,
  SetResponseNormal& output)
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
  if (value != kSetResponseTag) {
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
  return RequireNoTrailingBytes(reader);
}

ApduStatus EncodeSetResponseNormal(
  const SetResponseNormal& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(kSetResponseTag);
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
  return writer.WriteU8(input.result);
}

ApduStatus DecodeSetRequest(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  SetRequest& output)
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
  if (tag != kSetRequestTag) {
    return ApduStatus::InvalidTag;
  }
  std::uint8_t choice = 0;
  status = reader.ReadU8(choice);
  if (status != ApduStatus::Ok) {
    return status;
  }
  output.choice = static_cast<SetRequestChoice>(choice);
  status = reader.ReadU8(output.invokeIdAndPriority);
  if (status != ApduStatus::Ok) {
    return status;
  }

  switch (choice) {
    case kNormalChoice:
      status = DecodeDescriptorWithSelection(reader, maximumDataDepth, output.normal);
      if (status != ApduStatus::Ok) {
        return status;
      }
      status = DecodeDlmsDataFromReader(reader, maximumDataDepth, output.data);
      break;

    case kWithFirstDataBlockChoice:
      status = DecodeDescriptorWithSelection(reader, maximumDataDepth, output.normal);
      if (status != ApduStatus::Ok) {
        return status;
      }
      status = DecodeDataBlockSA(reader, output.dataBlock);
      break;

    case kWithDataBlockChoice:
      status = DecodeDataBlockSA(reader, output.dataBlock);
      break;

    case kWithListChoice:
    case kWithListAndFirstDataBlockChoice:
      {
        std::size_t count = 0;
        status = ReadAxdrLength(reader, count);
        if (status != ApduStatus::Ok) {
          return status;
        }
        output.list.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
          CosemAttributeDescriptorWithSelection item = {};
          status = DecodeDescriptorWithSelection(reader, maximumDataDepth, item);
          if (status != ApduStatus::Ok) {
            return status;
          }
          output.list.push_back(item);
        }
        if (choice == kWithListChoice) {
          std::size_t valueCount = 0;
          status = ReadAxdrLength(reader, valueCount);
          if (status != ApduStatus::Ok) {
            return status;
          }
          if (valueCount != count) {
            return ApduStatus::InvalidLength;
          }
          output.valueList.reserve(valueCount);
          for (std::size_t i = 0; i < valueCount; ++i) {
            DlmsData value = {};
            status = DecodeDlmsDataFromReader(reader, maximumDataDepth, value);
            if (status != ApduStatus::Ok) {
              return status;
            }
            output.valueList.push_back(value);
          }
        } else {
          status = DecodeDataBlockSA(reader, output.dataBlock);
        }
      }
      break;

    default:
      return ApduStatus::UnsupportedXdlmsService;
  }
  if (status != ApduStatus::Ok) {
    return status;
  }
  return RequireNoTrailingBytes(reader);
}

ApduStatus EncodeSetRequest(
  const SetRequest& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(kSetRequestTag);
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
    case SetRequestChoice::Normal:
      status = EncodeDescriptorWithSelection(input.normal, writer);
      if (status != ApduStatus::Ok) {
        return status;
      }
      return EncodeDlmsData(input.data, writer);

    case SetRequestChoice::WithFirstDataBlock:
      status = EncodeDescriptorWithSelection(input.normal, writer);
      if (status != ApduStatus::Ok) {
        return status;
      }
      return EncodeDataBlockSA(input.dataBlock, writer);

    case SetRequestChoice::WithDataBlock:
      return EncodeDataBlockSA(input.dataBlock, writer);

    case SetRequestChoice::WithList:
      if (input.list.size() != input.valueList.size()) {
        return ApduStatus::InvalidLength;
      }
      status = WriteAxdrLength(writer, input.list.size());
      if (status != ApduStatus::Ok) {
        return status;
      }
      for (std::size_t i = 0; i < input.list.size(); ++i) {
        status = EncodeDescriptorWithSelection(input.list[i], writer);
        if (status != ApduStatus::Ok) {
          return status;
        }
      }
      status = WriteAxdrLength(writer, input.valueList.size());
      if (status != ApduStatus::Ok) {
        return status;
      }
      for (std::size_t i = 0; i < input.valueList.size(); ++i) {
        status = EncodeDlmsData(input.valueList[i], writer);
        if (status != ApduStatus::Ok) {
          return status;
        }
      }
      return ApduStatus::Ok;

    case SetRequestChoice::WithListAndFirstDataBlock:
      status = WriteAxdrLength(writer, input.list.size());
      if (status != ApduStatus::Ok) {
        return status;
      }
      for (std::size_t i = 0; i < input.list.size(); ++i) {
        status = EncodeDescriptorWithSelection(input.list[i], writer);
        if (status != ApduStatus::Ok) {
          return status;
        }
      }
      return EncodeDataBlockSA(input.dataBlock, writer);
  }
  return ApduStatus::InvalidChoice;
}

ApduStatus DecodeSetResponse(
  const std::uint8_t* input,
  std::size_t inputSize,
  SetResponse& output)
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
  if (tag != kSetResponseTag) {
    return ApduStatus::InvalidTag;
  }
  std::uint8_t choice = 0;
  status = reader.ReadU8(choice);
  if (status != ApduStatus::Ok) {
    return status;
  }
  output.choice = static_cast<SetResponseChoice>(choice);
  status = reader.ReadU8(output.invokeIdAndPriority);
  if (status != ApduStatus::Ok) {
    return status;
  }

  switch (choice) {
    case kNormalChoice:
      status = reader.ReadU8(output.result);
      break;

    case kWithFirstDataBlockChoice:
      status = reader.ReadU32(output.blockNumber);
      break;

    case kWithDataBlockChoice:
      status = reader.ReadU8(output.result);
      if (status != ApduStatus::Ok) {
        return status;
      }
      status = reader.ReadU32(output.blockNumber);
      break;

    case kWithListChoice:
      status = DecodeResultList(reader, output.resultList);
      if (status != ApduStatus::Ok) {
        return status;
      }
      status = reader.ReadU32(output.blockNumber);
      break;

    case kWithListAndFirstDataBlockChoice:
      status = DecodeResultList(reader, output.resultList);
      break;

    default:
      return ApduStatus::UnsupportedXdlmsService;
  }
  if (status != ApduStatus::Ok) {
    return status;
  }
  return RequireNoTrailingBytes(reader);
}

ApduStatus EncodeSetResponse(
  const SetResponse& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(kSetResponseTag);
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
    case SetResponseChoice::Normal:
      return writer.WriteU8(input.result);

    case SetResponseChoice::DataBlock:
      return writer.WriteU32(input.blockNumber);

    case SetResponseChoice::LastDataBlock:
      status = writer.WriteU8(input.result);
      if (status != ApduStatus::Ok) {
        return status;
      }
      return writer.WriteU32(input.blockNumber);

    case SetResponseChoice::LastDataBlockWithList:
      status = EncodeResultList(input.resultList, writer);
      if (status != ApduStatus::Ok) {
        return status;
      }
      return writer.WriteU32(input.blockNumber);

    case SetResponseChoice::WithList:
      return EncodeResultList(input.resultList, writer);
  }
  return ApduStatus::InvalidChoice;
}

} // namespace apdu
} // namespace dlms
