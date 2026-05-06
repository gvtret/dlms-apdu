# APDU codec test plan

## Strategy

Tests are layered in the same order as implementation:

```text
status/error model
byte reader/writer
BER
A-XDR
Initiate
ACSE
Data
GET
SET
ACTION
top-level xDLMS dispatch
C ABI
cross-layer integration
```

Each codec module needs positive roundtrip tests and negative malformed/truncated input tests.

## BER tests

```text
DecodeBer_shortLength
DecodeBer_longLength
DecodeBer_nestedSequence
DecodeBer_objectIdentifier
DecodeBer_rejectsIndefiniteLength
DecodeBer_rejectsLengthBeyondInput
EncodeBer_roundtrip
DecodeBer_depthLimit
```

## A-XDR tests

```text
DecodeAxdr_optionalAbsent
DecodeAxdr_optionalPresent
DecodeAxdr_defaultAbsent
DecodeAxdr_unsignedValues
DecodeAxdr_octetString
DecodeAxdr_conformanceApplicationTag
EncodeAxdr_roundtrip
DecodeAxdr_rejectsTruncatedLength
```

## Initiate tests

```text
DecodeInitiateRequest_greenBookLnVector
EncodeInitiateRequest_greenBookLnVector
DecodeInitiateResponse_greenBookVector
EncodeInitiateResponse_greenBookVector
RejectInitiateRequest_missingConformance
RejectInitiateResponse_missingVaaName
```

## ACSE tests

```text
DecodeAarq_withUserInformation
EncodeAarq_roundtrip
DecodeAare_acceptWithInitiateResponse
DecodeAare_rejectWithConfirmedServiceError
DecodeRlrq_emptyReleaseRequest
EncodeRlrq_emptyReleaseRequest
DecodeRlre_withReleaseReason
EncodeRlre_withReleaseReason
DecodeAcse_dispatchReleaseApdus
DecodeAarq_withAuthenticationValue
RejectAarq_invalidUserInformationOctetString
RejectAcse_truncatedBer
RejectAcse_unsupportedIndefiniteLength
```

## Data tests

```text
DecodeData_null
DecodeData_boolean
DecodeData_unsigned
DecodeData_octetString
DecodeData_visibleString
DecodeData_dateTime
DecodeData_array
DecodeData_structure
RejectData_unknownTag
RejectData_compactArrayUnsupported
RejectData_depthLimit
RejectData_elementCountLimit
```

## GET tests

```text
DecodeGetRequestNormal_spodesVector
EncodeGetRequestNormal_spodesVector
DecodeGetResponseNormal_spodesVector
EncodeGetResponseNormal_spodesVector
DecodeGetResponseWithDataBlock
DecodeGetRequestWithList
RejectGetRequest_truncatedDescriptor
RejectGetRequest_invalidSelectiveAccessFlag
```

## SET tests

```text
EncodeSetRequestNormal_roundtrip
DecodeSetResponseNormal
EncodeSetRequestWithFirstDataBlock
DecodeSetResponseLastDataBlock
EncodeSetRequestWithList_roundtrip
RejectSetRequest_truncatedData
RejectSetResponse_unknownResult
```

## ACTION tests

```text
EncodeActionRequestNormal_roundtrip
DecodeActionResponseNormal
EncodeActionRequestNextPblock
DecodeActionResponseWithPblock
EncodeActionRequestWithList_roundtrip
RejectActionRequest_truncatedMethodDescriptor
RejectActionResponse_invalidChoice
```

## Top-level dispatch tests

```text
DecodeXdlms_dispatchInitiateRequest
DecodeXdlms_dispatchGetRequest
DecodeXdlms_dispatchSetRequest
DecodeXdlms_dispatchActionRequest
DecodeXdlms_dispatchOpaqueCiphered
DecodeXdlms_rejectUnsupportedKnownTag
DecodeXdlms_rejectUnknownTag
```

## C ABI tests

```text
CApi_headerCompilesAsC
CApi_statusValuesAreStable
CApi_decodeXdlmsKind
CApi_decodeAcseKind
CApi_encodeXdlmsRaw
CApi_outputBufferTooSmall
CApi_noCrashOnNullArguments
```

## Cross-layer integration tests

Root integration tests live in `E:/work/dlms/test/integration`.

Required scenarios:

```text
APDU -> LLC -> HDLC -> LLC -> APDU for AARQ
APDU -> LLC -> HDLC -> LLC -> APDU for GET request
APDU -> WRAPPER -> APDU for AARQ when wrapper codec API is available
APDU -> WRAPPER -> APDU for GET request when wrapper codec API is available
SPOdes trace HDLC frame -> LLC -> APDU for AARQ
SPOdes trace HDLC frame -> LLC -> APDU for AARE
SPOdes trace HDLC frame -> LLC -> APDU for GET request
payload byte 0x7E inside APDU survives lower layers
```

The APDU integration target must be gated by `DLMS_APDU_HAS_CODEC_API` and `TARGET dlms_apdu` until the APDU codec exists.
