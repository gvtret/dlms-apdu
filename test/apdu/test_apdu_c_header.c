#include "dlms/apdu/apdu_c_api.h"

int dlms_apdu_c_header_compiles_as_c(void)
{
  dlms_apdu_xdlms_t apdu;
  apdu.kind = DLMS_APDU_XDLMS_GET_REQUEST;
  apdu.tag = 0;
  apdu.payload = 0;
  apdu.payload_size = 0;
  return (int)DLMS_APDU_STATUS_OK + (int)apdu.kind;
}
