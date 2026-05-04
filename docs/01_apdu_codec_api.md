# APDU C++ codec API

## API principles

The C++ API is C++11-compatible and status-code based.

Public functions must:

```text
return ApduStatus
avoid exceptions in public API contracts
accept explicit input sizes
validate null pointers when size is non-zero
preserve unsupported but recognized ciphered payloads as opaque bytes
```

## Core types

```cpp
namespace dlms {
namespace apdu {

struct ByteView
{
  const std::uint8_t* data;
  std::size_t size;
};

struct ApduCodecLimits
{
  std::size_t maximumApduSize;
  std::size_t maximumBerDepth;
  std::size_t maximumAxdrDepth;
  std::size_t maximumArrayElements;
  std::size_t maximumStructureElements;
  std::size_t maximumOctetStringSize;
  std::size_t maximumRawDataBlockSize;
};

ApduCodecLimits DefaultApduCodecLimits();

} // namespace apdu
} // namespace dlms
```

`ByteView` is non-owning. Callers own the referenced bytes and must keep them alive for the duration of the call.

## Top-level APDU model

```cpp
enum class AcseApduKind
{
  Aarq,
  Aare,
  Rlrq,
  Rlre
};

enum class XdlmsApduKind
{
  InitiateRequest,
  InitiateResponse,
  ConfirmedServiceError,
  GetRequest,
  GetResponse,
  SetRequest,
  SetResponse,
  ActionRequest,
  ActionResponse,
  DataNotification,
  ExceptionResponse,
  GeneralGloCiphering,
  GeneralDedCiphering,
  GeneralCiphering,
  UnknownCiphered
};
```

The first implemented model can use explicit structs per service. Avoid a speculative inheritance hierarchy. Add ownership only where tests and use cases require it.

## Encoding APIs

High-level API:

```cpp
ApduStatus EncodeAcseApdu(
  const AcseApdu& apdu,
  std::vector<std::uint8_t>& output);

ApduStatus EncodeXdlmsApdu(
  const XdlmsApdu& apdu,
  std::vector<std::uint8_t>& output);
```

Strict API:

```cpp
ApduStatus EncodeAcseApduToBuffer(
  const AcseApdu& apdu,
  const ApduCodecLimits& limits,
  std::uint8_t* output,
  std::size_t outputSize,
  std::size_t& writtenSize);

ApduStatus EncodeXdlmsApduToBuffer(
  const XdlmsApdu& apdu,
  const ApduCodecLimits& limits,
  std::uint8_t* output,
  std::size_t outputSize,
  std::size_t& writtenSize);
```

## Decoding APIs

High-level API:

```cpp
ApduStatus DecodeAcseApdu(
  const std::uint8_t* input,
  std::size_t inputSize,
  AcseApdu& output);

ApduStatus DecodeXdlmsApdu(
  const std::uint8_t* input,
  std::size_t inputSize,
  XdlmsApdu& output);
```

Strict API can decode to caller-provided storage where practical. Complex Data trees may initially stay C++ owning containers; C ABI can expose narrower raw decode entry points first.

## Helper APIs

The initial helper surface should be small:

```cpp
XdlmsApdu MakeDefaultInitiateRequest();

XdlmsApdu MakeGetRequestNormal(
  std::uint8_t invokeIdAndPriority,
  std::uint16_t classId,
  const LogicalName& logicalName,
  std::uint8_t attributeId);

AcseApdu MakeAarqWithInitiateRequest(
  const XdlmsApdu& initiateRequest);
```

Helpers should encode common valid APDUs only. They must not hide policy decisions like authentication mechanism selection or negotiated conformance.

## Descriptor types

LN descriptors use fixed-size values:

```cpp
struct LogicalName
{
  std::uint8_t value[6];
};

struct CosemAttributeDescriptor
{
  std::uint16_t classId;
  LogicalName instanceId;
  std::uint8_t attributeId;
};

struct CosemMethodDescriptor
{
  std::uint16_t classId;
  LogicalName instanceId;
  std::uint8_t methodId;
};
```

Class ids are encoded as big-endian unsigned 16-bit values. Logical name is exactly six bytes.

