#include "dlms/apdu/xdlms.hpp"

#include "dlms/apdu/apdu_writer.hpp"

namespace dlms {
namespace apdu {

namespace {

constexpr std::uint8_t kInitiateRequestTag = 0x01;
constexpr std::uint8_t kInitiateResponseTag = 0x08;
constexpr std::uint8_t kGetRequestTag = 0xC0;
constexpr std::uint8_t kGetResponseTag = 0xC4;

ApduStatus WriteVectorResult(
  ApduStatus status,
  const std::uint8_t* buffer,
  std::size_t writtenSize,
  std::vector<std::uint8_t>& output)
{
  if (status != ApduStatus::Ok) {
    return status;
  }
  output.assign(buffer, buffer + writtenSize);
  return ApduStatus::Ok;
}

} // namespace

LogicalName::LogicalName(
  std::uint8_t b0,
  std::uint8_t b1,
  std::uint8_t b2,
  std::uint8_t b3,
  std::uint8_t b4,
  std::uint8_t b5)
{
  bytes[0] = b0;
  bytes[1] = b1;
  bytes[2] = b2;
  bytes[3] = b3;
  bytes[4] = b4;
  bytes[5] = b5;
}

XdlmsApdu::XdlmsApdu()
  : kind(XdlmsApduKind::InitiateRequest)
  , initiateRequest(MakeDefaultInitiateRequest())
  , initiateResponse()
  , getRequest()
  , getResponse()
{
}

XdlmsApdu::XdlmsApdu(const InitiateRequest& request)
  : kind(XdlmsApduKind::InitiateRequest)
  , initiateRequest(request)
  , initiateResponse()
  , getRequest()
  , getResponse()
{
}

XdlmsApdu MakeGetRequestNormal(
  std::uint8_t invokeIdAndPriority,
  std::uint16_t classId,
  const LogicalName& logicalName,
  std::uint8_t attributeId)
{
  XdlmsApdu apdu;
  apdu.kind = XdlmsApduKind::GetRequest;
  apdu.getRequest.invokeIdAndPriority = invokeIdAndPriority;
  apdu.getRequest.descriptor.classId = classId;
  for (std::size_t i = 0; i < 6; ++i) {
    apdu.getRequest.descriptor.logicalName[i] = logicalName.bytes[i];
  }
  apdu.getRequest.descriptor.attributeId = attributeId;
  apdu.getRequest.hasSelectiveAccess = false;
  return apdu;
}

ApduStatus DecodeXdlmsApdu(
  const std::uint8_t* input,
  std::size_t inputSize,
  XdlmsApdu& output)
{
  if (input == nullptr || inputSize == 0) {
    return inputSize == 0 ? ApduStatus::NeedMoreData : ApduStatus::InvalidArgument;
  }

  output = {};
  switch (input[0]) {
    case kInitiateRequestTag:
      output.kind = XdlmsApduKind::InitiateRequest;
      return DecodeInitiateRequest(input, inputSize, output.initiateRequest);

    case kInitiateResponseTag:
      output.kind = XdlmsApduKind::InitiateResponse;
      return DecodeInitiateResponse(input, inputSize, output.initiateResponse);

    case kGetRequestTag:
      output.kind = XdlmsApduKind::GetRequest;
      return DecodeGetRequestNormal(input, inputSize, output.getRequest);

    case kGetResponseTag:
      output.kind = XdlmsApduKind::GetResponse;
      return DecodeGetResponseNormal(input, inputSize, 8, output.getResponse);

    default:
      return ApduStatus::UnsupportedXdlmsService;
  }
}

ApduStatus EncodeXdlmsApdu(
  const XdlmsApdu& input,
  std::vector<std::uint8_t>& output)
{
  std::uint8_t buffer[2048] = {};
  ApduWriter writer(buffer, sizeof(buffer));
  ApduStatus status = ApduStatus::InternalError;

  switch (input.kind) {
    case XdlmsApduKind::InitiateRequest:
      status = EncodeInitiateRequest(input.initiateRequest, writer);
      break;

    case XdlmsApduKind::InitiateResponse:
      status = EncodeInitiateResponse(input.initiateResponse, writer);
      break;

    case XdlmsApduKind::GetRequest:
      status = EncodeGetRequestNormal(input.getRequest, writer);
      break;

    case XdlmsApduKind::GetResponse:
      status = EncodeGetResponseNormal(input.getResponse, writer);
      break;
  }

  return WriteVectorResult(status, buffer, writer.WrittenSize(), output);
}

} // namespace apdu
} // namespace dlms
