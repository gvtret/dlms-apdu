#include "dlms/apdu/get.hpp"

#include "dlms/apdu/apdu_reader.hpp"

namespace dlms {
namespace apdu {

namespace {

constexpr std::uint8_t kGetRequestTag = 0xC0;
constexpr std::uint8_t kGetResponseTag = 0xC4;
constexpr std::uint8_t kNormalChoice = 0x01;

ApduStatus RequireNoTrailingBytes(ApduReader& reader)
{
  return reader.Empty() ? ApduStatus::Ok : ApduStatus::InvalidLength;
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
    return ApduStatus::UnsupportedFeature;
  }

  return RequireNoTrailingBytes(reader);
}

ApduStatus EncodeGetRequestNormal(
  const GetRequestNormal& input,
  ApduWriter& writer)
{
  if (input.hasSelectiveAccess) {
    return ApduStatus::UnsupportedFeature;
  }

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
  return writer.WriteU8(0x00);
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

} // namespace apdu
} // namespace dlms
