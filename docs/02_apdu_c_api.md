# APDU C ABI

## Goals

The C ABI is a stable wrapper over the C++ implementation. It uses only C
types, fixed enum values and caller-provided buffers.

The ABI does not expose recursive C++ DATA trees. It provides a raw xDLMS APDU
view first, which keeps the ABI small while the C++ typed model continues to
evolve.

## Header

```text
include/dlms/apdu/apdu_c_api.h
```

The header includes only `<stddef.h>` and `<stdint.h>` and is valid as C.

## Status Values

`dlms_apdu_status_t` mirrors `ApduStatus` and keeps stable numeric values:

```text
DLMS_APDU_STATUS_OK = 0
DLMS_APDU_STATUS_NEED_MORE_DATA = 1
DLMS_APDU_STATUS_OUTPUT_BUFFER_TOO_SMALL = 2
DLMS_APDU_STATUS_INVALID_ARGUMENT = 3
DLMS_APDU_STATUS_INVALID_TAG = 4
DLMS_APDU_STATUS_INVALID_LENGTH = 5
DLMS_APDU_STATUS_INVALID_BER = 6
DLMS_APDU_STATUS_INVALID_AXDR = 7
DLMS_APDU_STATUS_INVALID_CHOICE = 8
DLMS_APDU_STATUS_INVALID_DATA = 9
DLMS_APDU_STATUS_INVALID_INVOKE_ID = 10
DLMS_APDU_STATUS_INVALID_DESCRIPTOR = 11
DLMS_APDU_STATUS_INVALID_CONFORMANCE = 12
DLMS_APDU_STATUS_UNSUPPORTED_APDU = 13
DLMS_APDU_STATUS_UNSUPPORTED_ACSE_FIELD = 14
DLMS_APDU_STATUS_UNSUPPORTED_XDLMS_SERVICE = 15
DLMS_APDU_STATUS_UNSUPPORTED_DATA_TYPE = 16
DLMS_APDU_STATUS_UNSUPPORTED_FEATURE = 17
DLMS_APDU_STATUS_PDU_TOO_LARGE = 18
DLMS_APDU_STATUS_INTERNAL_ERROR = 19
```

## xDLMS View

```c
typedef struct dlms_apdu_xdlms_t
{
  dlms_apdu_xdlms_kind_t kind;
  uint8_t tag;
  const uint8_t* payload;
  size_t payload_size;
} dlms_apdu_xdlms_t;
```

`payload` is non-owning. After `dlms_apdu_decode_xdlms`, it points into the
input buffer after the top-level APDU tag. The caller must keep the input bytes
alive while the view is used.

For encoding, `payload` is copied during the call. The encoder does not retain
the pointer after returning.

## Functions

```c
dlms_apdu_status_t dlms_apdu_decode_xdlms(
  const uint8_t* input,
  size_t input_size,
  dlms_apdu_xdlms_t* output);
```

Decodes enough of an xDLMS APDU to identify the top-level kind and expose the
raw payload. The C++ typed decoder is used internally for validation.

```c
dlms_apdu_status_t dlms_apdu_encode_xdlms(
  const dlms_apdu_xdlms_t* input,
  uint8_t* output,
  size_t output_size,
  size_t* written_size);
```

Encodes `tag + payload` into the caller-provided output buffer.

## Validation

The C ABI returns:

```text
INVALID_ARGUMENT for null required pointers
INVALID_ARGUMENT for null input with non-zero input_size
NEED_MORE_DATA for empty input decode
INVALID_ARGUMENT for null payload with non-zero payload_size
OUTPUT_BUFFER_TOO_SMALL when output_size is insufficient
INTERNAL_ERROR for unexpected C++ exceptions
```

`written_size` is set to zero before encode validation and remains zero on
errors.

## Tests

C ABI coverage includes:

```text
header compiles as C
raw xDLMS decode view
raw xDLMS encode view
small output buffer
null argument validation
```
