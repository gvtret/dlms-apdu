# APDU codec requirements

## Scope

`dlms-apdu` implements a portable C++11 codec for DLMS/COSEM application-layer APDUs.

The v1 codec covers:

```text
ACSE AARQ/AARE/RLRQ/RLRE APDU encoded with BER
xDLMS APDU encoded with A-XDR
xDLMS InitiateRequest and InitiateResponse
ConfirmedServiceError
LN GET request/response APDU
LN SET request/response APDU
LN ACTION request/response APDU
DLMS Data values used by those services
opaque ciphered APDU variants
```

The codec does not implement:

```text
COSEM object model
association state machine
authentication algorithm execution
AES-GCM ciphering
key management
HDLC, LLC, WRAPPER, or any transport
retry, timeout, or retransmission policy
SN referencing behavior
XML schema support
```

## Encoding boundaries

The APDU layer has two explicit encoding domains:

```text
ACSE APDUs: BER
xDLMS APDUs: A-XDR
```

`AARQ`, `AARE`, `RLRQ`, and `RLRE` are encoded as BER APDUs. The `user-information` field of `AARQ` contains an xDLMS `InitiateRequest` encoded in A-XDR and carried as an OCTET STRING. The `user-information` field of `AARE` contains an xDLMS `InitiateResponse` or `ConfirmedServiceError` encoded in A-XDR and carried as an OCTET STRING.

BER and A-XDR helpers must stay separate modules. The bridge between them belongs in the ACSE codec only.

## Error model

Public APIs return `ApduStatus`. They must not throw exceptions, call `abort`, or rely on runtime `assert`.

The status model must distinguish:

```text
Ok
NeedMoreData
OutputBufferTooSmall
InvalidArgument
InvalidTag
InvalidLength
InvalidBer
InvalidAxdr
InvalidChoice
InvalidData
InvalidInvokeId
InvalidDescriptor
InvalidConformance
UnsupportedApdu
UnsupportedAcseField
UnsupportedXdlmsService
UnsupportedDataType
UnsupportedFeature
PduTooLarge
InternalError
```

`NeedMoreData` is reserved for truncated input. Malformed complete input must return a specific invalid/unsupported status.

## Memory and buffer policy

The library exposes two C++ API levels:

```text
high-level vector API for application code and tests
strict caller-provided buffer API for C ABI and predictable-memory users
```

The strict API must write only into caller-provided buffers and must report `OutputBufferTooSmall` before writing partial inconsistent output.

## Limits

All decoders must enforce configurable limits:

```text
maximum APDU size
maximum BER nesting depth
maximum A-XDR nesting depth
maximum array elements
maximum structure elements
maximum OCTET STRING size
maximum raw data block size
```

Limits apply to vector APIs and caller-provided buffer APIs.

## Ciphered APDU handling

v1 recognizes ciphered APDU tags and returns their content as opaque bytes.

The codec must not:

```text
decrypt ciphered content
verify authentication tags
require keys
infer security policy
```

Security execution belongs to a future module.

## Block transfer boundary

GET, SET, and ACTION block APDU forms are encoded and decoded as data structures. The codec does not decide when to split data into blocks, request the next block, retry, or validate block sequencing as a protocol state machine.

## Cross-layer boundary

LLC, HDLC, and WRAPPER treat APDU bytes as opaque payload. `dlms-apdu` must not include or depend on lower-layer headers.

