#ifndef DLMS_APDU_APDU_C_API_H
#define DLMS_APDU_APDU_C_API_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum dlms_apdu_status_t
{
  DLMS_APDU_STATUS_OK = 0,
  DLMS_APDU_STATUS_NEED_MORE_DATA = 1,
  DLMS_APDU_STATUS_OUTPUT_BUFFER_TOO_SMALL = 2,
  DLMS_APDU_STATUS_INVALID_ARGUMENT = 3,
  DLMS_APDU_STATUS_INVALID_TAG = 4,
  DLMS_APDU_STATUS_INVALID_LENGTH = 5,
  DLMS_APDU_STATUS_INVALID_BER = 6,
  DLMS_APDU_STATUS_INVALID_AXDR = 7,
  DLMS_APDU_STATUS_INVALID_CHOICE = 8,
  DLMS_APDU_STATUS_INVALID_DATA = 9,
  DLMS_APDU_STATUS_INVALID_INVOKE_ID = 10,
  DLMS_APDU_STATUS_INVALID_DESCRIPTOR = 11,
  DLMS_APDU_STATUS_INVALID_CONFORMANCE = 12,
  DLMS_APDU_STATUS_UNSUPPORTED_APDU = 13,
  DLMS_APDU_STATUS_UNSUPPORTED_ACSE_FIELD = 14,
  DLMS_APDU_STATUS_UNSUPPORTED_XDLMS_SERVICE = 15,
  DLMS_APDU_STATUS_UNSUPPORTED_DATA_TYPE = 16,
  DLMS_APDU_STATUS_UNSUPPORTED_FEATURE = 17,
  DLMS_APDU_STATUS_PDU_TOO_LARGE = 18,
  DLMS_APDU_STATUS_INTERNAL_ERROR = 19
} dlms_apdu_status_t;

typedef enum dlms_apdu_xdlms_kind_t
{
  DLMS_APDU_XDLMS_INITIATE_REQUEST = 0,
  DLMS_APDU_XDLMS_INITIATE_RESPONSE = 1,
  DLMS_APDU_XDLMS_GET_REQUEST = 2,
  DLMS_APDU_XDLMS_GET_RESPONSE = 3,
  DLMS_APDU_XDLMS_SET_REQUEST = 4,
  DLMS_APDU_XDLMS_SET_RESPONSE = 5,
  DLMS_APDU_XDLMS_ACTION_REQUEST = 6,
  DLMS_APDU_XDLMS_ACTION_RESPONSE = 7,
  DLMS_APDU_XDLMS_CIPHERED = 8
} dlms_apdu_xdlms_kind_t;

typedef struct dlms_apdu_xdlms_t
{
  dlms_apdu_xdlms_kind_t kind;
  uint8_t tag;
  const uint8_t* payload;
  size_t payload_size;
} dlms_apdu_xdlms_t;

dlms_apdu_status_t dlms_apdu_decode_xdlms(
  const uint8_t* input,
  size_t input_size,
  dlms_apdu_xdlms_t* output);

dlms_apdu_status_t dlms_apdu_encode_xdlms(
  const dlms_apdu_xdlms_t* input,
  uint8_t* output,
  size_t output_size,
  size_t* written_size);

#ifdef __cplusplus
}
#endif

#endif /* DLMS_APDU_APDU_C_API_H */
