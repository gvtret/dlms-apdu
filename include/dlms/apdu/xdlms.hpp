#ifndef DLMS_APDU_XDLMS_HPP
#define DLMS_APDU_XDLMS_HPP

#include "dlms/apdu/apdu_error.hpp"
#include "dlms/apdu/apdu_types.hpp"
#include "dlms/apdu/action.hpp"
#include "dlms/apdu/get.hpp"
#include "dlms/apdu/initiate.hpp"
#include "dlms/apdu/set.hpp"

#include <cstdint>
#include <vector>

namespace dlms {
namespace apdu {

struct LogicalName
{
  std::uint8_t bytes[6];

  LogicalName(
    std::uint8_t b0,
    std::uint8_t b1,
    std::uint8_t b2,
    std::uint8_t b3,
    std::uint8_t b4,
    std::uint8_t b5);
};

enum class XdlmsApduKind
{
  InitiateRequest,
  InitiateResponse,
  GetRequest,
  GetResponse,
  SetRequest,
  SetResponse,
  ActionRequest,
  ActionResponse,
  Ciphered
};

enum class CipheredApduKind
{
  ServiceSpecific,
  GeneralGloCiphering,
  GeneralDedCiphering,
  GeneralCiphering
};

struct CipheredApdu
{
  CipheredApduKind kind;
  std::uint8_t tag;
  ByteView payload;
};

struct XdlmsApdu
{
  XdlmsApduKind kind;
  InitiateRequest initiateRequest;
  InitiateResponse initiateResponse;
  GetRequestNormal getRequest;
  GetResponseNormal getResponse;
  SetRequestNormal setRequest;
  SetResponseNormal setResponse;
  ActionRequestNormal actionRequest;
  ActionResponseNormal actionResponse;
  GetRequest getRequestAny;
  GetResponse getResponseAny;
  SetRequest setRequestAny;
  SetResponse setResponseAny;
  ActionRequest actionRequestAny;
  ActionResponse actionResponseAny;
  CipheredApdu ciphered;

  XdlmsApdu();
  XdlmsApdu(const InitiateRequest& request);
};

XdlmsApdu MakeGetRequestNormal(
  std::uint8_t invokeIdAndPriority,
  std::uint16_t classId,
  const LogicalName& logicalName,
  std::uint8_t attributeId);

ApduStatus DecodeXdlmsApdu(
  const std::uint8_t* input,
  std::size_t inputSize,
  XdlmsApdu& output);

ApduStatus EncodeXdlmsApdu(
  const XdlmsApdu& input,
  std::vector<std::uint8_t>& output);

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_XDLMS_HPP
