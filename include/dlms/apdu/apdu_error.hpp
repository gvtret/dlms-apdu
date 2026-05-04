#ifndef DLMS_APDU_APDU_ERROR_HPP
#define DLMS_APDU_APDU_ERROR_HPP

namespace dlms {
namespace apdu {

enum class ApduStatus
{
  Ok = 0,

  NeedMoreData,
  OutputBufferTooSmall,

  InvalidArgument,
  InvalidTag,
  InvalidLength,
  InvalidBer,
  InvalidAxdr,
  InvalidChoice,
  InvalidData,
  InvalidInvokeId,
  InvalidDescriptor,
  InvalidConformance,

  UnsupportedApdu,
  UnsupportedAcseField,
  UnsupportedXdlmsService,
  UnsupportedDataType,
  UnsupportedFeature,

  PduTooLarge,
  InternalError
};

const char* ApduStatusName(ApduStatus status);

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_APDU_ERROR_HPP
