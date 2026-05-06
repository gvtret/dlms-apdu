#include "dlms/apdu/acse.hpp"

#include "dlms/apdu/apdu_reader.hpp"
#include "dlms/apdu/ber.hpp"
#include "dlms/apdu/xdlms.hpp"

namespace dlms {
namespace apdu {

namespace {

constexpr std::uint8_t kAarqTag = 0x60;
constexpr std::uint8_t kAareTag = 0x61;
constexpr std::uint8_t kRlrqTag = 0x62;
constexpr std::uint8_t kRlreTag = 0x63;
constexpr std::uint8_t kUserInformationTag = 0xBE;
constexpr std::uint8_t kOctetStringTag = 0x04;
constexpr std::uint8_t kAssociationResultTag = 0xA2;
constexpr std::uint8_t kResultSourceDiagnosticTag = 0xA3;
constexpr std::uint8_t kAcseServiceUserTag = 0xA1;
constexpr std::uint8_t kBerIntegerTag = 0x02;
constexpr std::uint8_t kReleaseReasonTag = 0x80;

ByteView MakeView(const std::uint8_t* data, std::size_t size)
{
  ByteView view = {};
  view.data = data;
  view.size = size;
  return view;
}

ApduStatus ReadSingleTlv(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::uint8_t expectedTag,
  BerTlv& tlv)
{
  if (input == nullptr && inputSize != 0) {
    return ApduStatus::InvalidArgument;
  }

  ApduReader reader(input, inputSize);
  ApduStatus status = ReadBerTlv(reader, tlv);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (tlv.tag != expectedTag) {
    return ApduStatus::InvalidTag;
  }
  if (!reader.Empty()) {
    return ApduStatus::InvalidLength;
  }
  return ApduStatus::Ok;
}

ApduStatus ReadUserInformation(BerTlv userInformation, BerTlv& octetString)
{
  if (userInformation.tag != kUserInformationTag) {
    return ApduStatus::InvalidTag;
  }

  ApduReader reader(userInformation.value, userInformation.valueSize);
  ApduStatus status = ReadBerTlv(reader, octetString);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (octetString.tag != kOctetStringTag) {
    return ApduStatus::InvalidTag;
  }
  if (!reader.Empty()) {
    return ApduStatus::InvalidLength;
  }
  return ApduStatus::Ok;
}

ApduStatus ReadIntegerValue(BerTlv integerTlv, std::int32_t& value)
{
  if (integerTlv.tag != kBerIntegerTag) {
    return ApduStatus::InvalidTag;
  }
  if (integerTlv.valueSize == 0 || integerTlv.valueSize > 4) {
    return ApduStatus::InvalidLength;
  }

  std::int32_t result = 0;
  for (std::size_t i = 0; i < integerTlv.valueSize; ++i) {
    result = (result << 8) | integerTlv.value[i];
  }
  if ((integerTlv.value[0] & 0x80U) != 0U && integerTlv.valueSize < 4) {
    result |= static_cast<std::int32_t>(-1) << (integerTlv.valueSize * 8U);
  }
  value = result;
  return ApduStatus::Ok;
}

ApduStatus ReadPrimitiveIntegerValue(BerTlv integerTlv, std::int32_t& value)
{
  if (integerTlv.valueSize == 0 || integerTlv.valueSize > 4) {
    return ApduStatus::InvalidLength;
  }

  std::int32_t result = 0;
  for (std::size_t i = 0; i < integerTlv.valueSize; ++i) {
    result = (result << 8) | integerTlv.value[i];
  }
  if ((integerTlv.value[0] & 0x80U) != 0U && integerTlv.valueSize < 4) {
    result |= static_cast<std::int32_t>(-1) << (integerTlv.valueSize * 8U);
  }
  value = result;
  return ApduStatus::Ok;
}

ApduStatus ReadNestedInteger(
  BerTlv outer,
  std::uint8_t optionalInnerTag,
  std::int32_t& value)
{
  ApduReader outerReader(outer.value, outer.valueSize);
  BerTlv first = {};
  ApduStatus status = ReadBerTlv(outerReader, first);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (!outerReader.Empty()) {
    return ApduStatus::InvalidLength;
  }

  if (first.tag == kBerIntegerTag) {
    return ReadIntegerValue(first, value);
  }
  if (first.tag != optionalInnerTag) {
    return ApduStatus::InvalidTag;
  }

  ApduReader innerReader(first.value, first.valueSize);
  BerTlv integerTlv = {};
  status = ReadBerTlv(innerReader, integerTlv);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (!innerReader.Empty()) {
    return ApduStatus::InvalidLength;
  }
  return ReadIntegerValue(integerTlv, value);
}

ApduStatus CopyRawField(const AcseRawField& field, ApduWriter& writer)
{
  if (field.encoded.data == nullptr && field.encoded.size != 0) {
    return ApduStatus::InvalidArgument;
  }
  return writer.WriteBytes(field.encoded.data, field.encoded.size);
}

ApduStatus WriteUserInformation(
  const std::uint8_t* xdlms,
  std::size_t xdlmsSize,
  ApduWriter& writer)
{
  std::uint8_t octetString[512] = {};
  ApduWriter octetWriter(octetString, sizeof(octetString));
  ApduStatus status = WriteBerTlv(octetWriter, kOctetStringTag, xdlms, xdlmsSize);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return WriteBerTlv(
    writer,
    kUserInformationTag,
    octetString,
    octetWriter.WrittenSize());
}

template <typename T>
ApduStatus EncodeAssociation(
  std::uint8_t tag,
  const T& input,
  const std::uint8_t* xdlms,
  std::size_t xdlmsSize,
  ApduWriter& writer)
{
  std::uint8_t value[1024] = {};
  ApduWriter valueWriter(value, sizeof(value));
  for (std::size_t i = 0; i < input.fields.size(); ++i) {
    if (input.fields[i].tag == kUserInformationTag) {
      return ApduStatus::InvalidArgument;
    }
    ApduStatus status = CopyRawField(input.fields[i], valueWriter);
    if (status != ApduStatus::Ok) {
      return status;
    }
  }

  ApduStatus status = WriteUserInformation(xdlms, xdlmsSize, valueWriter);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return WriteBerTlv(writer, tag, value, valueWriter.WrittenSize());
}

ApduStatus WriteReleaseReason(std::int32_t reason, ApduWriter& writer)
{
  if (reason < -128 || reason > 127) {
    return ApduStatus::UnsupportedFeature;
  }

  const std::uint8_t value = static_cast<std::uint8_t>(reason & 0xff);
  return WriteBerTlv(writer, kReleaseReasonTag, &value, 1);
}

template <typename T>
ApduStatus DecodeRelease(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::uint8_t expectedTag,
  T& output)
{
  output = {};

  BerTlv release = {};
  ApduStatus status = ReadSingleTlv(input, inputSize, expectedTag, release);
  if (status != ApduStatus::Ok) {
    return status;
  }

  ApduReader reader(release.value, release.valueSize);
  while (!reader.Empty()) {
    const std::size_t elementStart = reader.Position();
    BerTlv child = {};
    status = ReadBerTlv(reader, child);
    if (status != ApduStatus::Ok) {
      return status;
    }
    const std::size_t elementSize = reader.Position() - elementStart;

    if (child.tag == kReleaseReasonTag) {
      status = ReadPrimitiveIntegerValue(child, output.reason);
      if (status != ApduStatus::Ok) {
        return status;
      }
      output.hasReason = true;
    }

    output.fields.push_back(
      AcseRawField{child.tag, MakeView(release.value + elementStart, elementSize)});
  }

  return ApduStatus::Ok;
}

template <typename T>
ApduStatus EncodeRelease(
  std::uint8_t tag,
  const T& input,
  ApduWriter& writer)
{
  std::uint8_t value[256] = {};
  ApduWriter valueWriter(value, sizeof(value));
  for (std::size_t i = 0; i < input.fields.size(); ++i) {
    if (input.fields[i].tag == kReleaseReasonTag && input.hasReason) {
      continue;
    }
    ApduStatus status = CopyRawField(input.fields[i], valueWriter);
    if (status != ApduStatus::Ok) {
      return status;
    }
  }

  if (input.hasReason) {
    const ApduStatus status = WriteReleaseReason(input.reason, valueWriter);
    if (status != ApduStatus::Ok) {
      return status;
    }
  }

  return WriteBerTlv(writer, tag, value, valueWriter.WrittenSize());
}

} // namespace

ApduStatus DecodeAarq(
  const std::uint8_t* input,
  std::size_t inputSize,
  AarqApdu& output)
{
  output = {};

  BerTlv aarq = {};
  ApduStatus status = ReadSingleTlv(input, inputSize, kAarqTag, aarq);
  if (status != ApduStatus::Ok) {
    return status;
  }

  bool hasUserInformation = false;
  ApduReader reader(aarq.value, aarq.valueSize);
  while (!reader.Empty()) {
    const std::size_t elementStart = reader.Position();
    BerTlv child = {};
    status = ReadBerTlv(reader, child);
    if (status != ApduStatus::Ok) {
      return status;
    }
    const std::size_t elementSize = reader.Position() - elementStart;
    if (child.tag != kUserInformationTag) {
      output.fields.push_back(
        AcseRawField{child.tag, MakeView(aarq.value + elementStart, elementSize)});
      continue;
    }

    BerTlv octetString = {};
    status = ReadUserInformation(child, octetString);
    if (status != ApduStatus::Ok) {
      return status;
    }
    status = DecodeInitiateRequest(
      octetString.value,
      octetString.valueSize,
      output.initiateRequest);
    if (status != ApduStatus::Ok) {
      return status;
    }
    hasUserInformation = true;
  }

  return hasUserInformation ? ApduStatus::Ok : ApduStatus::InvalidData;
}

ApduStatus EncodeAarq(
  const AarqApdu& input,
  ApduWriter& writer)
{
  std::uint8_t xdlms[256] = {};
  ApduWriter xdlmsWriter(xdlms, sizeof(xdlms));
  ApduStatus status = EncodeInitiateRequest(input.initiateRequest, xdlmsWriter);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return EncodeAssociation(
    kAarqTag,
    input,
    xdlms,
    xdlmsWriter.WrittenSize(),
    writer);
}

ApduStatus DecodeAare(
  const std::uint8_t* input,
  std::size_t inputSize,
  AareApdu& output)
{
  output = {};

  BerTlv aare = {};
  ApduStatus status = ReadSingleTlv(input, inputSize, kAareTag, aare);
  if (status != ApduStatus::Ok) {
    return status;
  }

  bool hasUserInformation = false;
  ApduReader reader(aare.value, aare.valueSize);
  while (!reader.Empty()) {
    const std::size_t elementStart = reader.Position();
    BerTlv child = {};
    status = ReadBerTlv(reader, child);
    if (status != ApduStatus::Ok) {
      return status;
    }
    const std::size_t elementSize = reader.Position() - elementStart;
    if (child.tag == kAssociationResultTag) {
      status = ReadNestedInteger(child, kBerIntegerTag, output.result);
      if (status != ApduStatus::Ok) {
        return status;
      }
      output.hasResult = true;
    } else if (child.tag == kResultSourceDiagnosticTag) {
      status = ReadNestedInteger(child, kAcseServiceUserTag, output.diagnostic);
      if (status != ApduStatus::Ok) {
        return status;
      }
      output.hasDiagnostic = true;
    }

    if (child.tag != kUserInformationTag) {
      output.fields.push_back(
        AcseRawField{child.tag, MakeView(aare.value + elementStart, elementSize)});
      continue;
    }

    BerTlv octetString = {};
    status = ReadUserInformation(child, octetString);
    if (status != ApduStatus::Ok) {
      return status;
    }
    status = DecodeInitiateResponse(
      octetString.value,
      octetString.valueSize,
      output.initiateResponse);
    if (status != ApduStatus::Ok) {
      return status;
    }
    hasUserInformation = true;
  }

  return hasUserInformation ? ApduStatus::Ok : ApduStatus::InvalidData;
}

ApduStatus EncodeAare(
  const AareApdu& input,
  ApduWriter& writer)
{
  std::uint8_t xdlms[256] = {};
  ApduWriter xdlmsWriter(xdlms, sizeof(xdlms));
  ApduStatus status = EncodeInitiateResponse(input.initiateResponse, xdlmsWriter);
  if (status != ApduStatus::Ok) {
    return status;
  }
  return EncodeAssociation(
    kAareTag,
    input,
    xdlms,
    xdlmsWriter.WrittenSize(),
    writer);
}

ApduStatus DecodeRlrq(
  const std::uint8_t* input,
  std::size_t inputSize,
  RlrqApdu& output)
{
  return DecodeRelease(input, inputSize, kRlrqTag, output);
}

ApduStatus EncodeRlrq(
  const RlrqApdu& input,
  ApduWriter& writer)
{
  return EncodeRelease(kRlrqTag, input, writer);
}

ApduStatus DecodeRlre(
  const std::uint8_t* input,
  std::size_t inputSize,
  RlreApdu& output)
{
  return DecodeRelease(input, inputSize, kRlreTag, output);
}

ApduStatus EncodeRlre(
  const RlreApdu& input,
  ApduWriter& writer)
{
  return EncodeRelease(kRlreTag, input, writer);
}

AcseApdu MakeAarqWithInitiateRequest(const XdlmsApdu& initiateRequest)
{
  AcseApdu apdu = {};
  apdu.kind = AcseApduKind::Aarq;
  apdu.aarq.initiateRequest = initiateRequest.initiateRequest;
  return apdu;
}

AcseApdu MakeRlrq()
{
  AcseApdu apdu = {};
  apdu.kind = AcseApduKind::Rlrq;
  return apdu;
}

ApduStatus DecodeAcseApdu(
  const std::uint8_t* input,
  std::size_t inputSize,
  AcseApdu& output)
{
  if (input == nullptr || inputSize == 0) {
    return inputSize == 0 ? ApduStatus::NeedMoreData : ApduStatus::InvalidArgument;
  }

  output = {};
  if (input[0] == kAarqTag) {
    output.kind = AcseApduKind::Aarq;
    return DecodeAarq(input, inputSize, output.aarq);
  }
  if (input[0] == kAareTag) {
    output.kind = AcseApduKind::Aare;
    return DecodeAare(input, inputSize, output.aare);
  }
  if (input[0] == kRlrqTag) {
    output.kind = AcseApduKind::Rlrq;
    return DecodeRlrq(input, inputSize, output.rlrq);
  }
  if (input[0] == kRlreTag) {
    output.kind = AcseApduKind::Rlre;
    return DecodeRlre(input, inputSize, output.rlre);
  }
  return ApduStatus::InvalidTag;
}

ApduStatus EncodeAcseApdu(
  const AcseApdu& input,
  std::vector<std::uint8_t>& output)
{
  std::uint8_t buffer[2048] = {};
  ApduWriter writer(buffer, sizeof(buffer));
  ApduStatus status = ApduStatus::InternalError;

  switch (input.kind) {
    case AcseApduKind::Aarq:
      status = EncodeAarq(input.aarq, writer);
      break;

    case AcseApduKind::Aare:
      status = EncodeAare(input.aare, writer);
      break;

    case AcseApduKind::Rlrq:
      status = EncodeRlrq(input.rlrq, writer);
      break;

    case AcseApduKind::Rlre:
      status = EncodeRlre(input.rlre, writer);
      break;
  }

  if (status != ApduStatus::Ok) {
    return status;
  }
  output.assign(buffer, buffer + writer.WrittenSize());
  return ApduStatus::Ok;
}

} // namespace apdu
} // namespace dlms
