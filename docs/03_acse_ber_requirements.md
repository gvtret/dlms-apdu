# ACSE BER requirements

## Scope

The BER module supports only the BER subset needed by DLMS/COSEM ACSE APDUs in v1.

Supported APDUs:

```text
AARQ
AARE
RLRQ
RLRE
```

## Required BER features

The BER reader/writer must support:

```text
definite short-form length
definite long-form length
constructed tags
context-specific tags
application tags used by ACSE APDUs
OBJECT IDENTIFIER
INTEGER
ENUMERATED
BIT STRING
OCTET STRING
NULL
SEQUENCE
```

Indefinite length is not supported in v1. Decoder must return `UnsupportedFeature` or `InvalidBer` consistently; tests must lock the chosen status.

## ACSE field coverage

v1 model fields:

```text
application-context-name
sender-acse-requirements
responder-acse-requirements
mechanism-name
calling-authentication-value
responding-authentication-value
result
result-source-diagnostic
user-information
```

Unsupported optional fields must not be silently re-encoded as if they were understood. Decoder behavior must be explicit:

```text
skip and report metadata, or
return UnsupportedAcseField
```

The implementation phase must choose one behavior before coding full AARQ/AARE decode.

## user-information bridge

For `AARQ`, `user-information` carries an A-XDR encoded xDLMS `InitiateRequest` as BER OCTET STRING.

For `AARE`, `user-information` carries an A-XDR encoded xDLMS `InitiateResponse` or `ConfirmedServiceError` as BER OCTET STRING.

The ACSE codec owns this bridge. BER helpers must not parse xDLMS content.

## Validation requirements

Decoder must reject:

```text
truncated tag
truncated length
declared length beyond input
unsupported indefinite length
invalid primitive/constructed form for known fields
invalid user-information wrapper
```

Encoder must reject:

```text
missing mandatory AARQ application-context-name
missing mandatory AARE result
missing mandatory AARE result-source-diagnostic
ACSE APDU larger than configured maximumApduSize
```

## Test vectors

Required vectors:

```text
Green Book AARQ examples
Green Book AARE success examples
Green Book AARE failure examples
SPOdes AARQ HDLC trace after HDLC and LLC decode
SPOdes AARE HDLC trace after HDLC and LLC decode
```

