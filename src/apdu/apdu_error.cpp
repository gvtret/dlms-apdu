#include "dlms/apdu/apdu_error.hpp"

namespace dlms {
namespace apdu {

const char* ApduStatusName(ApduStatus status)
{
  switch (status) {
    case ApduStatus::Ok:
      return "Ok";
    case ApduStatus::NeedMoreData:
      return "NeedMoreData";
    case ApduStatus::OutputBufferTooSmall:
      return "OutputBufferTooSmall";
    case ApduStatus::InvalidArgument:
      return "InvalidArgument";
    case ApduStatus::InvalidTag:
      return "InvalidTag";
    case ApduStatus::InvalidLength:
      return "InvalidLength";
    case ApduStatus::InvalidBer:
      return "InvalidBer";
    case ApduStatus::InvalidAxdr:
      return "InvalidAxdr";
    case ApduStatus::InvalidChoice:
      return "InvalidChoice";
    case ApduStatus::InvalidData:
      return "InvalidData";
    case ApduStatus::InvalidInvokeId:
      return "InvalidInvokeId";
    case ApduStatus::InvalidDescriptor:
      return "InvalidDescriptor";
    case ApduStatus::InvalidConformance:
      return "InvalidConformance";
    case ApduStatus::UnsupportedApdu:
      return "UnsupportedApdu";
    case ApduStatus::UnsupportedAcseField:
      return "UnsupportedAcseField";
    case ApduStatus::UnsupportedXdlmsService:
      return "UnsupportedXdlmsService";
    case ApduStatus::UnsupportedDataType:
      return "UnsupportedDataType";
    case ApduStatus::UnsupportedFeature:
      return "UnsupportedFeature";
    case ApduStatus::PduTooLarge:
      return "PduTooLarge";
    case ApduStatus::InternalError:
      return "InternalError";
  }

  return "Unknown";
}

} // namespace apdu
} // namespace dlms
