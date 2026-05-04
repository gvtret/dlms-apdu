#include "dlms/apdu/get.hpp"

#include "dlms/apdu/apdu_reader.hpp"
#include "dlms/apdu/axdr.hpp"

namespace dlms {
namespace apdu {

namespace {

constexpr std::uint8_t kGetRequestTag = 0xC0;
constexpr std::uint8_t kGetResponseTag = 0xC4;
constexpr std::uint8_t kNormalChoice = 0x01;
constexpr std::uint8_t kNextChoice = 0x02;
constexpr std::uint8_t kWithListChoice = 0x03;

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

ApduStatus DecodeGetDataResult(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  GetDataResult& output)
{
  ApduReader reader(input, inputSize);
  std::uint8_t choice = 0;
  ApduStatus status = reader.ReadU8(choice);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (choice > 1U) {
    return ApduStatus::InvalidChoice;
  }
  output.choice = static_cast<GetDataResultChoice>(choice);
  if (output.choice == GetDataResultChoice::Data) {
    status = DecodeDlmsDataFromReader(reader, maximumDataDepth, output.data);
    if (status != ApduStatus::Ok) {
      return status;
    }
    return RequireNoTrailingBytes(reader);
  }
  status = reader.ReadU8(output.dataAccessError);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return RequireNoTrailingBytes(reader);
}

ApduStatus EncodeGetDataResult(const GetDataResult& input, ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(static_cast<std::uint8_t>(input.choice));
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (input.choice == GetDataResultChoice::Data) {
    return EncodeDlmsData(input.data, writer);
  }
  if (input.choice == GetDataResultChoice::DataAccessError) {
    return writer.WriteU8(input.dataAccessError);
  }
  return ApduStatus::InvalidChoice;
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

} // namespace

ApduStatus DecodeGetRequestNormal(
  const std::uint8_t* input,
  std::size_t inputSize,
  GetRequestNormal& output)
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
  if (value != kGetRequestTag) {
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

  status = reader.ReadU8(value);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (value > 1U) {
    return ApduStatus::InvalidChoice;
  }
  output.hasSelectiveAccess = value != 0U;
  if (output.hasSelectiveAccess) {
    status = reader.ReadU8(output.selectiveAccess.selector);
    if (status != ApduStatus::Ok) {
      return status;
    }
    status = DecodeDlmsDataFromReader(reader, 8, output.selectiveAccess.parameters);
    if (status != ApduStatus::Ok) {
      return status;
    }
    return RequireNoTrailingBytes(reader);
  }

  return RequireNoTrailingBytes(reader);
}

ApduStatus EncodeGetRequestNormal(
  const GetRequestNormal& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(kGetRequestTag);
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
  return EncodeSelection(input.hasSelectiveAccess, input.selectiveAccess, writer);
}

ApduStatus DecodeGetResponseNormal(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  GetResponseNormal& output)
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
  if (value != kGetResponseTag) {
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

  status = reader.ReadU8(value);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (value > 1U) {
    return ApduStatus::InvalidChoice;
  }
  output.resultChoice = static_cast<GetDataResultChoice>(value);

  if (output.resultChoice == GetDataResultChoice::Data) {
    const std::uint8_t* data = nullptr;
    const std::size_t dataSize = reader.Remaining();
    status = reader.ReadBytes(data, dataSize);
    if (status != ApduStatus::Ok) {
      return status;
    }
    return DecodeDlmsData(data, dataSize, maximumDataDepth, output.data);
  }

  status = reader.ReadU8(output.dataAccessError);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return RequireNoTrailingBytes(reader);
}

ApduStatus DecodeGetRequest(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  GetRequest& output)
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
  if (tag != kGetRequestTag) {
    return ApduStatus::InvalidTag;
  }
  std::uint8_t choice = 0;
  status = reader.ReadU8(choice);
  if (status != ApduStatus::Ok) {
    return status;
  }
  output.choice = static_cast<GetRequestChoice>(choice);
  status = reader.ReadU8(output.invokeIdAndPriority);
  if (status != ApduStatus::Ok) {
    return status;
  }

  switch (choice) {
    case kNormalChoice:
      status = DecodeDescriptorWithSelection(reader, maximumDataDepth, output.normal);
      break;

    case kNextChoice:
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
          CosemAttributeDescriptorWithSelection item = {};
          status = DecodeDescriptorWithSelection(reader, maximumDataDepth, item);
          if (status != ApduStatus::Ok) {
            return status;
          }
          output.list.push_back(item);
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

ApduStatus EncodeGetRequest(
  const GetRequest& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(kGetRequestTag);
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
    case GetRequestChoice::Normal:
      return EncodeDescriptorWithSelection(input.normal, writer);

    case GetRequestChoice::Next:
      return writer.WriteU32(input.blockNumber);

    case GetRequestChoice::WithList:
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
      return ApduStatus::Ok;
  }
  return ApduStatus::InvalidChoice;
}

ApduStatus EncodeGetResponseNormal(
  const GetResponseNormal& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(kGetResponseTag);
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
  status = writer.WriteU8(static_cast<std::uint8_t>(input.resultChoice));
  if (status != ApduStatus::Ok) {
    return status;
  }

  if (input.resultChoice == GetDataResultChoice::Data) {
    return EncodeDlmsData(input.data, writer);
  }
  if (input.resultChoice == GetDataResultChoice::DataAccessError) {
    return writer.WriteU8(input.dataAccessError);
  }
  return ApduStatus::InvalidChoice;
}

ApduStatus DecodeGetResponse(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  GetResponse& output)
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
  if (tag != kGetResponseTag) {
    return ApduStatus::InvalidTag;
  }
  std::uint8_t choice = 0;
  status = reader.ReadU8(choice);
  if (status != ApduStatus::Ok) {
    return status;
  }
  output.choice = static_cast<GetResponseChoice>(choice);
  status = reader.ReadU8(output.invokeIdAndPriority);
  if (status != ApduStatus::Ok) {
    return status;
  }
  switch (choice) {
    case kNormalChoice:
      {
        const std::uint8_t* result = nullptr;
        const std::size_t resultSize = reader.Remaining();
        status = reader.ReadBytes(result, resultSize);
        if (status != ApduStatus::Ok) {
          return status;
        }
        return DecodeGetDataResult(result, resultSize, maximumDataDepth, output.result);
      }

    case kNextChoice:
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
          const std::size_t start = reader.Position();
          GetDataResult result = {};
          if (reader.Remaining() == 0) {
            return ApduStatus::NeedMoreData;
          }
          std::uint8_t resultChoice = 0;
          status = reader.ReadU8(resultChoice);
          if (status != ApduStatus::Ok) {
            return status;
          }
          if (resultChoice == 0) {
            status = DecodeDlmsDataFromReader(reader, maximumDataDepth, result.data);
            if (status != ApduStatus::Ok) {
              return status;
            }
            result.choice = GetDataResultChoice::Data;
          } else if (resultChoice == 1) {
            result.choice = GetDataResultChoice::DataAccessError;
            status = reader.ReadU8(result.dataAccessError);
            if (status != ApduStatus::Ok) {
              return status;
            }
          } else {
            return ApduStatus::InvalidChoice;
          }
          if (reader.Position() == start) {
            return ApduStatus::InternalError;
          }
          output.list.push_back(result);
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

ApduStatus EncodeGetResponse(
  const GetResponse& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(kGetResponseTag);
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
    case GetResponseChoice::Normal:
      return EncodeGetDataResult(input.result, writer);

    case GetResponseChoice::WithDataBlock:
      return EncodeDataBlockG(input.dataBlock, writer);

    case GetResponseChoice::WithList:
      status = WriteAxdrLength(writer, input.list.size());
      if (status != ApduStatus::Ok) {
        return status;
      }
      for (std::size_t i = 0; i < input.list.size(); ++i) {
        status = EncodeGetDataResult(input.list[i], writer);
        if (status != ApduStatus::Ok) {
          return status;
        }
      }
      return ApduStatus::Ok;
  }
  return ApduStatus::InvalidChoice;
}

} // namespace apdu
} // namespace dlms
