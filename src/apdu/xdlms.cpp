#include "dlms/apdu/xdlms.hpp"

#include "dlms/apdu/apdu_writer.hpp"

namespace dlms {
namespace apdu {

namespace {

constexpr std::uint8_t kInitiateRequestTag = 0x01;
constexpr std::uint8_t kInitiateResponseTag = 0x08;
constexpr std::uint8_t kGetRequestTag = 0xC0;
constexpr std::uint8_t kSetRequestTag = 0xC1;
constexpr std::uint8_t kActionRequestTag = 0xC3;
constexpr std::uint8_t kGetResponseTag = 0xC4;
constexpr std::uint8_t kSetResponseTag = 0xC5;
constexpr std::uint8_t kActionResponseTag = 0xC7;
constexpr std::uint8_t kGloGetRequestTag = 0xC8;
constexpr std::uint8_t kGloSetRequestTag = 0xC9;
constexpr std::uint8_t kGloEventNotificationRequestTag = 0xCA;
constexpr std::uint8_t kGloActionRequestTag = 0xCB;
constexpr std::uint8_t kGloGetResponseTag = 0xCC;
constexpr std::uint8_t kGloSetResponseTag = 0xCD;
constexpr std::uint8_t kGloActionResponseTag = 0xCF;
constexpr std::uint8_t kDedGetRequestTag = 0xD0;
constexpr std::uint8_t kDedSetRequestTag = 0xD1;
constexpr std::uint8_t kDedEventNotificationRequestTag = 0xD2;
constexpr std::uint8_t kDedActionRequestTag = 0xD3;
constexpr std::uint8_t kDedGetResponseTag = 0xD4;
constexpr std::uint8_t kDedSetResponseTag = 0xD5;
constexpr std::uint8_t kDedActionResponseTag = 0xD7;
constexpr std::uint8_t kGeneralGloCipheringTag = 0xDB;
constexpr std::uint8_t kGeneralDedCipheringTag = 0xDC;
constexpr std::uint8_t kGeneralCipheringTag = 0xDD;

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

bool IsGetRequestChoice(GetRequestChoice choice)
{
  return choice == GetRequestChoice::Normal ||
    choice == GetRequestChoice::Next ||
    choice == GetRequestChoice::WithList;
}

bool IsGetResponseChoice(GetResponseChoice choice)
{
  return choice == GetResponseChoice::Normal ||
    choice == GetResponseChoice::WithDataBlock ||
    choice == GetResponseChoice::WithList;
}

bool IsSetRequestChoice(SetRequestChoice choice)
{
  return choice == SetRequestChoice::Normal ||
    choice == SetRequestChoice::WithFirstDataBlock ||
    choice == SetRequestChoice::WithDataBlock ||
    choice == SetRequestChoice::WithList ||
    choice == SetRequestChoice::WithListAndFirstDataBlock;
}

bool IsSetResponseChoice(SetResponseChoice choice)
{
  return choice == SetResponseChoice::Normal ||
    choice == SetResponseChoice::DataBlock ||
    choice == SetResponseChoice::LastDataBlock ||
    choice == SetResponseChoice::LastDataBlockWithList ||
    choice == SetResponseChoice::WithList;
}

bool IsActionRequestChoice(ActionRequestChoice choice)
{
  return choice == ActionRequestChoice::Normal ||
    choice == ActionRequestChoice::NextPblock ||
    choice == ActionRequestChoice::WithList ||
    choice == ActionRequestChoice::WithFirstPblock ||
    choice == ActionRequestChoice::WithListAndFirstPblock ||
    choice == ActionRequestChoice::WithPblock;
}

bool IsActionResponseChoice(ActionResponseChoice choice)
{
  return choice == ActionResponseChoice::Normal ||
    choice == ActionResponseChoice::WithPblock ||
    choice == ActionResponseChoice::WithList ||
    choice == ActionResponseChoice::NextPblock;
}

bool IsServiceSpecificCipheredTag(std::uint8_t tag)
{
  return tag == kGloGetRequestTag ||
    tag == kGloSetRequestTag ||
    tag == kGloEventNotificationRequestTag ||
    tag == kGloActionRequestTag ||
    tag == kGloGetResponseTag ||
    tag == kGloSetResponseTag ||
    tag == kGloActionResponseTag ||
    tag == kDedGetRequestTag ||
    tag == kDedSetRequestTag ||
    tag == kDedEventNotificationRequestTag ||
    tag == kDedActionRequestTag ||
    tag == kDedGetResponseTag ||
    tag == kDedSetResponseTag ||
    tag == kDedActionResponseTag;
}

bool IsCipheredTag(std::uint8_t tag)
{
  return IsServiceSpecificCipheredTag(tag) ||
    tag == kGeneralGloCipheringTag ||
    tag == kGeneralDedCipheringTag ||
    tag == kGeneralCipheringTag;
}

CipheredApduKind CipheredKindFromTag(std::uint8_t tag)
{
  if (tag == kGeneralGloCipheringTag) {
    return CipheredApduKind::GeneralGloCiphering;
  }
  if (tag == kGeneralDedCipheringTag) {
    return CipheredApduKind::GeneralDedCiphering;
  }
  if (tag == kGeneralCipheringTag) {
    return CipheredApduKind::GeneralCiphering;
  }
  return CipheredApduKind::ServiceSpecific;
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
  , setRequest()
  , setResponse()
  , actionRequest()
  , actionResponse()
  , getRequestAny()
  , getResponseAny()
  , setRequestAny()
  , setResponseAny()
  , actionRequestAny()
  , actionResponseAny()
  , ciphered()
{
}

XdlmsApdu::XdlmsApdu(const InitiateRequest& request)
  : kind(XdlmsApduKind::InitiateRequest)
  , initiateRequest(request)
  , initiateResponse()
  , getRequest()
  , getResponse()
  , setRequest()
  , setResponse()
  , actionRequest()
  , actionResponse()
  , getRequestAny()
  , getResponseAny()
  , setRequestAny()
  , setResponseAny()
  , actionRequestAny()
  , actionResponseAny()
  , ciphered()
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
  apdu.getRequestAny.choice = GetRequestChoice::Normal;
  apdu.getRequestAny.invokeIdAndPriority = invokeIdAndPriority;
  apdu.getRequestAny.normal.descriptor = apdu.getRequest.descriptor;
  apdu.getRequestAny.normal.hasSelection = false;
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
      {
        ApduStatus status = DecodeGetRequest(input, inputSize, 8, output.getRequestAny);
        if (status != ApduStatus::Ok) {
          return status;
        }
        if (output.getRequestAny.choice == GetRequestChoice::Normal) {
          output.getRequest.invokeIdAndPriority = output.getRequestAny.invokeIdAndPriority;
          output.getRequest.descriptor = output.getRequestAny.normal.descriptor;
          output.getRequest.hasSelectiveAccess = output.getRequestAny.normal.hasSelection;
          output.getRequest.selectiveAccess = output.getRequestAny.normal.selection;
        }
        return ApduStatus::Ok;
      }

    case kGetResponseTag:
      output.kind = XdlmsApduKind::GetResponse;
      {
        ApduStatus status = DecodeGetResponse(input, inputSize, 8, output.getResponseAny);
        if (status != ApduStatus::Ok) {
          return status;
        }
        if (output.getResponseAny.choice == GetResponseChoice::Normal) {
          output.getResponse.invokeIdAndPriority = output.getResponseAny.invokeIdAndPriority;
          output.getResponse.resultChoice = output.getResponseAny.result.choice;
          output.getResponse.data = output.getResponseAny.result.data;
          output.getResponse.dataAccessError = output.getResponseAny.result.dataAccessError;
        }
        return ApduStatus::Ok;
      }

    case kSetRequestTag:
      output.kind = XdlmsApduKind::SetRequest;
      {
        ApduStatus status = DecodeSetRequest(input, inputSize, 8, output.setRequestAny);
        if (status != ApduStatus::Ok) {
          return status;
        }
        if (output.setRequestAny.choice == SetRequestChoice::Normal) {
          output.setRequest.invokeIdAndPriority = output.setRequestAny.invokeIdAndPriority;
          output.setRequest.descriptor = output.setRequestAny.normal.descriptor;
          output.setRequest.hasSelectiveAccess = output.setRequestAny.normal.hasSelection;
          output.setRequest.selectiveAccess = output.setRequestAny.normal.selection;
          output.setRequest.data = output.setRequestAny.data;
        }
        return ApduStatus::Ok;
      }

    case kSetResponseTag:
      output.kind = XdlmsApduKind::SetResponse;
      {
        ApduStatus status = DecodeSetResponse(input, inputSize, output.setResponseAny);
        if (status != ApduStatus::Ok) {
          return status;
        }
        if (output.setResponseAny.choice == SetResponseChoice::Normal) {
          output.setResponse.invokeIdAndPriority = output.setResponseAny.invokeIdAndPriority;
          output.setResponse.result = output.setResponseAny.result;
        }
        return ApduStatus::Ok;
      }

    case kActionRequestTag:
      output.kind = XdlmsApduKind::ActionRequest;
      {
        ApduStatus status = DecodeActionRequest(input, inputSize, 8, output.actionRequestAny);
        if (status != ApduStatus::Ok) {
          return status;
        }
        if (output.actionRequestAny.choice == ActionRequestChoice::Normal) {
          output.actionRequest.invokeIdAndPriority = output.actionRequestAny.invokeIdAndPriority;
          output.actionRequest.descriptor = output.actionRequestAny.normal.descriptor;
          output.actionRequest.hasInvocationParameter =
            output.actionRequestAny.normal.hasInvocationParameter;
          output.actionRequest.invocationParameter =
            output.actionRequestAny.normal.invocationParameter;
        }
        return ApduStatus::Ok;
      }

    case kActionResponseTag:
      output.kind = XdlmsApduKind::ActionResponse;
      {
        ApduStatus status = DecodeActionResponse(input, inputSize, 8, output.actionResponseAny);
        if (status != ApduStatus::Ok) {
          return status;
        }
        if (output.actionResponseAny.choice == ActionResponseChoice::Normal) {
          output.actionResponse.invokeIdAndPriority = output.actionResponseAny.invokeIdAndPriority;
          output.actionResponse.result = output.actionResponseAny.normal.result;
          output.actionResponse.hasReturnParameter =
            output.actionResponseAny.normal.hasReturnParameter;
          output.actionResponse.returnParameter = output.actionResponseAny.normal.returnParameter;
        }
        return ApduStatus::Ok;
      }

    default:
      if (IsCipheredTag(input[0])) {
        output.kind = XdlmsApduKind::Ciphered;
        output.ciphered.kind = CipheredKindFromTag(input[0]);
        output.ciphered.tag = input[0];
        output.ciphered.payload.data = input + 1;
        output.ciphered.payload.size = inputSize - 1U;
        return ApduStatus::Ok;
      }
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
      status = IsGetRequestChoice(input.getRequestAny.choice)
        ? EncodeGetRequest(input.getRequestAny, writer)
        : EncodeGetRequestNormal(input.getRequest, writer);
      break;

    case XdlmsApduKind::GetResponse:
      status = IsGetResponseChoice(input.getResponseAny.choice)
        ? EncodeGetResponse(input.getResponseAny, writer)
        : EncodeGetResponseNormal(input.getResponse, writer);
      break;

    case XdlmsApduKind::SetRequest:
      status = IsSetRequestChoice(input.setRequestAny.choice)
        ? EncodeSetRequest(input.setRequestAny, writer)
        : EncodeSetRequestNormal(input.setRequest, writer);
      break;

    case XdlmsApduKind::SetResponse:
      status = IsSetResponseChoice(input.setResponseAny.choice)
        ? EncodeSetResponse(input.setResponseAny, writer)
        : EncodeSetResponseNormal(input.setResponse, writer);
      break;

    case XdlmsApduKind::ActionRequest:
      status = IsActionRequestChoice(input.actionRequestAny.choice)
        ? EncodeActionRequest(input.actionRequestAny, writer)
        : EncodeActionRequestNormal(input.actionRequest, writer);
      break;

    case XdlmsApduKind::ActionResponse:
      status = IsActionResponseChoice(input.actionResponseAny.choice)
        ? EncodeActionResponse(input.actionResponseAny, writer)
        : EncodeActionResponseNormal(input.actionResponse, writer);
      break;

    case XdlmsApduKind::Ciphered:
      if (!IsCipheredTag(input.ciphered.tag)) {
        status = ApduStatus::UnsupportedXdlmsService;
        break;
      }
      status = writer.WriteU8(input.ciphered.tag);
      if (status == ApduStatus::Ok) {
        status = writer.WriteBytes(input.ciphered.payload.data, input.ciphered.payload.size);
      }
      break;
  }

  return WriteVectorResult(status, buffer, writer.WrittenSize(), output);
}

} // namespace apdu
} // namespace dlms
