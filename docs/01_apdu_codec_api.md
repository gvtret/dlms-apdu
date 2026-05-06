# APDU C++ codec API

## Scope

The C++ API is a C++11 codec layer for DLMS/COSEM ACSE and xDLMS APDUs.
It does not implement association state machines, object execution, security
algorithms, retries or transport policy.

Public functions return `ApduStatus`. They do not require exceptions for normal
error reporting.

## Ownership

`ByteView` is a non-owning byte view:

```cpp
struct ByteView
{
  const std::uint8_t* data;
  std::size_t size;
};
```

The caller owns the memory referenced by `ByteView`. Decoders may store
`ByteView` values pointing into the input APDU buffer, so the input bytes must
remain alive while the decoded object is inspected. Encoders copy bytes from
views during the call and do not retain them after returning.

`std::vector` fields in decoded models own their elements. Nested `ByteView`
fields inside those elements remain non-owning views into the original input.

## Error Statuses

All public codecs use `ApduStatus`:

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

Pointer rules are consistent across decoders: `input == nullptr` is valid only
when `inputSize == 0`. Empty input generally returns `NeedMoreData` for
top-level dispatch and reader-level APIs.

## ACSE API

`acse.hpp` exposes BER-encoded ACSE APDUs:

```cpp
ApduStatus DecodeAarq(const std::uint8_t* input, std::size_t inputSize, AarqApdu& output);
ApduStatus EncodeAarq(const AarqApdu& input, ApduWriter& writer);
ApduStatus DecodeAare(const std::uint8_t* input, std::size_t inputSize, AareApdu& output);
ApduStatus EncodeAare(const AareApdu& input, ApduWriter& writer);
ApduStatus DecodeRlrq(const std::uint8_t* input, std::size_t inputSize, RlrqApdu& output);
ApduStatus EncodeRlrq(const RlrqApdu& input, ApduWriter& writer);
ApduStatus DecodeRlre(const std::uint8_t* input, std::size_t inputSize, RlreApdu& output);
ApduStatus EncodeRlre(const RlreApdu& input, ApduWriter& writer);
AcseApdu MakeAarqWithInitiateRequest(const XdlmsApdu& initiateRequest);
AcseApdu MakeRlrq();
ApduStatus DecodeAcseApdu(const std::uint8_t* input, std::size_t inputSize, AcseApdu& output);
ApduStatus EncodeAcseApdu(const AcseApdu& input, std::vector<std::uint8_t>& output);
```

`DecodeAcseApdu` dispatches by ACSE tag. `EncodeAcseApdu` writes a complete
BER APDU into an owning vector.

RLRQ/RLRE are codec-only release APDUs. The APDU layer encodes and decodes
their BER envelopes and optional release reason fields; association state,
fallback close policy, and retries belong to higher layers.

## xDLMS API

`xdlms.hpp` exposes top-level xDLMS dispatch:

```cpp
XdlmsApdu MakeGetRequestNormal(
  std::uint8_t invokeIdAndPriority,
  std::uint16_t classId,
  const LogicalName& logicalName,
  std::uint8_t attributeId);

ApduStatus DecodeXdlmsApdu(const std::uint8_t* input, std::size_t inputSize, XdlmsApdu& output);
ApduStatus EncodeXdlmsApdu(const XdlmsApdu& input, std::vector<std::uint8_t>& output);
```

The dispatcher supports Initiate, GET, SET, ACTION and ciphered APDUs.
Ciphered APDUs are represented as opaque `CipheredApdu { kind, tag, payload }`
and are not decrypted.

## GET, SET And ACTION

GET supports normal, next and with-list requests, and normal, datablock and
with-list responses.

SET supports normal, first-datablock, datablock, with-list and
with-list-and-first-datablock requests, and all corresponding response choices.

ACTION supports normal, next-pblock, with-list, first-pblock,
list-and-first-pblock and pblock requests, plus normal, pblock, with-list and
next-pblock responses.

Normal legacy APIs are retained for simple callers:

```cpp
DecodeGetRequestNormal(...)
EncodeGetRequestNormal(...)
DecodeSetRequestNormal(...)
EncodeSetRequestNormal(...)
DecodeActionRequestNormal(...)
EncodeActionRequestNormal(...)
```

Generic APIs should be used when block transfer, selective access or list forms
are possible.

## Data API

`data.hpp` exposes A-XDR `Data` codec helpers:

```cpp
ApduStatus DecodeDlmsData(const std::uint8_t* input, std::size_t inputSize, std::size_t maximumDepth, DlmsData& output);
ApduStatus DecodeDlmsDataFromReader(ApduReader& reader, std::size_t maximumDepth, DlmsData& output);
ApduStatus EncodeDlmsData(const DlmsData& input, ApduWriter& writer);
```

`DecodeDlmsData` requires the whole input range to contain one complete DATA
value. `DecodeDlmsDataFromReader` consumes exactly one DATA value and is used by
list codecs.

## Limits

The current public API exposes per-call depth limits for DATA decoding. BER and
A-XDR primitive readers validate malformed length encodings and buffer
boundaries. Larger policy limits, such as maximum APDU size or session block
transfer policy, belong to future higher-level client/session code.
