# APDU C ABI requirements

## Goals

The C ABI is a stable wrapper over the C++ implementation.

It must support:

```text
C compilation
fixed enum values
fixed integer types
caller-provided buffers
no C++ types in public headers
no exceptions crossing the ABI
```

## Files

```text
include/dlms/apdu/apdu_c_api.h
src/apdu/apdu_c_api.cpp
test/apdu/test_apdu_c_api.cpp
```

## Status enum

The C status enum mirrors `ApduStatus` and keeps stable numeric values.

```c
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
  DLMS_APDU_STATUS_UNSUPPORTED_APDU = 8,
  DLMS_APDU_STATUS_UNSUPPORTED_FEATURE = 9,
  DLMS_APDU_STATUS_PDU_TOO_LARGE = 10,
  DLMS_APDU_STATUS_INTERNAL_ERROR = 11
} dlms_apdu_status_t;
```

The exact final list is fixed during status-model implementation and covered by tests.

## Minimal data structures

The first C ABI should avoid exposing a recursive Data tree until the C++ model is stable.

Use raw APDU views and small typed headers first:

```c
typedef struct dlms_apdu_byte_view_t
{
  const uint8_t* data;
  size_t size;
} dlms_apdu_byte_view_t;

typedef struct dlms_apdu_buffer_t
{
  uint8_t* data;
  size_t size;
  size_t written_size;
} dlms_apdu_buffer_t;
```

## Minimal functions

```c
dlms_apdu_status_t dlms_apdu_decode_xdlms_kind(
  const uint8_t* input,
  size_t input_size,
  uint32_t* kind);
```

```c
dlms_apdu_status_t dlms_apdu_encode_xdlms_raw(
  uint32_t kind,
  const uint8_t* body,
  size_t body_size,
  uint8_t* output,
  size_t output_size,
  size_t* written_size);
```

```c
dlms_apdu_status_t dlms_apdu_decode_acse_kind(
  const uint8_t* input,
  size_t input_size,
  uint32_t* kind);
```

More typed C functions can be added after the C++ model for Initiate and GET is implemented and tested.

## Validation rules

Every C ABI function must:

```text
return INVALID_ARGUMENT for null required output pointers
return INVALID_ARGUMENT for null input pointer with non-zero size
return OUTPUT_BUFFER_TOO_SMALL before writing past output_size
set written_size only on success or well-defined size-query paths
catch all C++ exceptions internally and convert them to INTERNAL_ERROR
```

## Tests

Required C ABI tests:

```text
header compiles as C
status values match documented order
decode rejects null input/output
encode rejects null output
small output buffer is reported
basic xDLMS kind decode works
basic ACSE kind decode works
```

