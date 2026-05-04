#include "dlms/apdu/apdu_c_api.h"

#include "dlms/apdu/xdlms.hpp"

#include <cstddef>

namespace {

dlms_apdu_status_t ToCApiStatus(dlms::apdu::ApduStatus status)
{
  switch (status) {
    case dlms::apdu::ApduStatus::Ok:
      return DLMS_APDU_STATUS_OK;
    case dlms::apdu::ApduStatus::NeedMoreData:
      return DLMS_APDU_STATUS_NEED_MORE_DATA;
    case dlms::apdu::ApduStatus::OutputBufferTooSmall:
      return DLMS_APDU_STATUS_OUTPUT_BUFFER_TOO_SMALL;
    case dlms::apdu::ApduStatus::InvalidArgument:
      return DLMS_APDU_STATUS_INVALID_ARGUMENT;
    case dlms::apdu::ApduStatus::InvalidTag:
      return DLMS_APDU_STATUS_INVALID_TAG;
    case dlms::apdu::ApduStatus::InvalidLength:
      return DLMS_APDU_STATUS_INVALID_LENGTH;
    case dlms::apdu::ApduStatus::InvalidBer:
      return DLMS_APDU_STATUS_INVALID_BER;
    case dlms::apdu::ApduStatus::InvalidAxdr:
      return DLMS_APDU_STATUS_INVALID_AXDR;
    case dlms::apdu::ApduStatus::InvalidChoice:
      return DLMS_APDU_STATUS_INVALID_CHOICE;
    case dlms::apdu::ApduStatus::InvalidData:
      return DLMS_APDU_STATUS_INVALID_DATA;
    case dlms::apdu::ApduStatus::InvalidInvokeId:
      return DLMS_APDU_STATUS_INVALID_INVOKE_ID;
    case dlms::apdu::ApduStatus::InvalidDescriptor:
      return DLMS_APDU_STATUS_INVALID_DESCRIPTOR;
    case dlms::apdu::ApduStatus::InvalidConformance:
      return DLMS_APDU_STATUS_INVALID_CONFORMANCE;
    case dlms::apdu::ApduStatus::UnsupportedApdu:
      return DLMS_APDU_STATUS_UNSUPPORTED_APDU;
    case dlms::apdu::ApduStatus::UnsupportedAcseField:
      return DLMS_APDU_STATUS_UNSUPPORTED_ACSE_FIELD;
    case dlms::apdu::ApduStatus::UnsupportedXdlmsService:
      return DLMS_APDU_STATUS_UNSUPPORTED_XDLMS_SERVICE;
    case dlms::apdu::ApduStatus::UnsupportedDataType:
      return DLMS_APDU_STATUS_UNSUPPORTED_DATA_TYPE;
    case dlms::apdu::ApduStatus::UnsupportedFeature:
      return DLMS_APDU_STATUS_UNSUPPORTED_FEATURE;
    case dlms::apdu::ApduStatus::PduTooLarge:
      return DLMS_APDU_STATUS_PDU_TOO_LARGE;
    case dlms::apdu::ApduStatus::InternalError:
      return DLMS_APDU_STATUS_INTERNAL_ERROR;
  }

  return DLMS_APDU_STATUS_INTERNAL_ERROR;
}

dlms_apdu_xdlms_kind_t ToCApiKind(dlms::apdu::XdlmsApduKind kind)
{
  switch (kind) {
    case dlms::apdu::XdlmsApduKind::InitiateRequest:
      return DLMS_APDU_XDLMS_INITIATE_REQUEST;
    case dlms::apdu::XdlmsApduKind::InitiateResponse:
      return DLMS_APDU_XDLMS_INITIATE_RESPONSE;
    case dlms::apdu::XdlmsApduKind::GetRequest:
      return DLMS_APDU_XDLMS_GET_REQUEST;
    case dlms::apdu::XdlmsApduKind::GetResponse:
      return DLMS_APDU_XDLMS_GET_RESPONSE;
    case dlms::apdu::XdlmsApduKind::SetRequest:
      return DLMS_APDU_XDLMS_SET_REQUEST;
    case dlms::apdu::XdlmsApduKind::SetResponse:
      return DLMS_APDU_XDLMS_SET_RESPONSE;
    case dlms::apdu::XdlmsApduKind::ActionRequest:
      return DLMS_APDU_XDLMS_ACTION_REQUEST;
    case dlms::apdu::XdlmsApduKind::ActionResponse:
      return DLMS_APDU_XDLMS_ACTION_RESPONSE;
    case dlms::apdu::XdlmsApduKind::Ciphered:
      return DLMS_APDU_XDLMS_CIPHERED;
  }

  return DLMS_APDU_XDLMS_CIPHERED;
}

} // namespace

extern "C" dlms_apdu_status_t dlms_apdu_decode_xdlms(
  const uint8_t* input,
  size_t input_size,
  dlms_apdu_xdlms_t* output)
{
  if (output == 0) {
    return DLMS_APDU_STATUS_INVALID_ARGUMENT;
  }

  output->kind = DLMS_APDU_XDLMS_INITIATE_REQUEST;
  output->tag = 0;
  output->payload = 0;
  output->payload_size = 0;

  if (input == 0 && input_size != 0) {
    return DLMS_APDU_STATUS_INVALID_ARGUMENT;
  }
  if (input_size == 0) {
    return DLMS_APDU_STATUS_NEED_MORE_DATA;
  }

  try {
    dlms::apdu::XdlmsApdu apdu;
    const dlms::apdu::ApduStatus status =
      dlms::apdu::DecodeXdlmsApdu(input, input_size, apdu);
    if (status != dlms::apdu::ApduStatus::Ok) {
      return ToCApiStatus(status);
    }

    output->kind = ToCApiKind(apdu.kind);
    output->tag = input[0];
    output->payload = input + 1;
    output->payload_size = input_size - 1U;
    return DLMS_APDU_STATUS_OK;
  } catch (...) {
    output->kind = DLMS_APDU_XDLMS_INITIATE_REQUEST;
    output->tag = 0;
    output->payload = 0;
    output->payload_size = 0;
    return DLMS_APDU_STATUS_INTERNAL_ERROR;
  }
}

extern "C" dlms_apdu_status_t dlms_apdu_encode_xdlms(
  const dlms_apdu_xdlms_t* input,
  uint8_t* output,
  size_t output_size,
  size_t* written_size)
{
  if (input == 0 || written_size == 0) {
    return DLMS_APDU_STATUS_INVALID_ARGUMENT;
  }

  *written_size = 0;
  if (input->payload == 0 && input->payload_size != 0) {
    return DLMS_APDU_STATUS_INVALID_ARGUMENT;
  }

  const size_t required_size = input->payload_size + 1U;
  if (output_size < required_size) {
    return DLMS_APDU_STATUS_OUTPUT_BUFFER_TOO_SMALL;
  }
  if (output == 0) {
    return DLMS_APDU_STATUS_INVALID_ARGUMENT;
  }

  output[0] = input->tag;
  for (size_t i = 0; i < input->payload_size; ++i) {
    output[i + 1U] = input->payload[i];
  }
  *written_size = required_size;
  return DLMS_APDU_STATUS_OK;
}
