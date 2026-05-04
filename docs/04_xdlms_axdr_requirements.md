# xDLMS A-XDR requirements

## Scope

The A-XDR module supports xDLMS APDUs used by DLMS/COSEM association and LN services in v1.

Covered APDU families:

```text
InitiateRequest
InitiateResponse
ConfirmedServiceError
GET request/response
SET request/response
ACTION request/response
DataNotification
ExceptionResponse
opaque ciphered APDU variants
```

## Required A-XDR primitives

The A-XDR reader/writer must support:

```text
CHOICE tag dispatch
OPTIONAL usage flag
DEFAULT usage flag
BOOLEAN
Unsigned8
Unsigned16
Unsigned32
Integer8
Integer16
Integer32
ENUMERATED
OCTET STRING with A-XDR length
fixed-order SEQUENCE
Conformance application tag 0x5f1f
```

## Initiate APDUs

`InitiateRequest` fields:

```text
dedicated-key OPTIONAL
response-allowed DEFAULT TRUE
proposed-quality-of-service OPTIONAL
proposed-dlms-version-number
proposed-conformance
client-max-receive-pdu-size
```

`InitiateResponse` fields:

```text
negotiated-quality-of-service OPTIONAL
negotiated-dlms-version-number
negotiated-conformance
server-max-receive-pdu-size
vaa-name
```

The conformance value is a 24-bit bit string. The Green Book LN example with conformance bytes `00 7E 1F` must be a test vector.

## LN service descriptors

COSEM attribute descriptor:

```text
class-id: Unsigned16
logical-name: 6 bytes
attribute-id: Unsigned8
```

COSEM method descriptor:

```text
class-id: Unsigned16
logical-name: 6 bytes
method-id: Unsigned8
```

## GET APDUs

Required forms:

```text
GetRequestNormal
GetRequestNext
GetRequestWithList
GetResponseNormal
GetResponseWithDataBlock
GetResponseWithList
```

## SET APDUs

Required forms:

```text
SetRequestNormal
SetRequestFirstDataBlock
SetRequestWithDataBlock
SetRequestWithList
SetRequestWithListAndFirstDataBlock
SetResponseNormal
SetResponseDataBlock
SetResponseLastDataBlock
SetResponseWithList
```

## ACTION APDUs

Required forms:

```text
ActionRequestNormal
ActionRequestNextPblock
ActionRequestWithList
ActionRequestWithFirstPblock
ActionRequestWithListAndFirstPblock
ActionRequestWithPblock
ActionResponseNormal
ActionResponseWithPblock
ActionResponseWithList
ActionResponseNextPblock
```

## Data codec

The Data codec supports typed DLMS values. It must not interpret class-specific semantics.

Required data tags:

```text
null-data
array
structure
boolean
bit-string
double-long
double-long-unsigned
octet-string
visible-string
utf8-string
bcd
integer
long
unsigned
long-unsigned
long64
long64-unsigned
enum
float32
float64
date-time
date
time
dont-care
```

`compact-array` is recognized but unsupported in v1.

## Ciphered APDUs

The decoder recognizes global, dedicated, and general ciphering tags and returns opaque bytes. It does not decrypt or authenticate them.

