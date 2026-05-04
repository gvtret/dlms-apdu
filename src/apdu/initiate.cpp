#include "dlms/apdu/initiate.hpp"

namespace dlms {
namespace apdu {

namespace {

ApduStatus ReadOptionalInteger8(
  ApduReader& reader,
  bool& present,
  std::int8_t& value)
{
  ApduStatus status = ReadAxdrOptionalFlag(reader, present);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (!present) {
    value = 0;
    return ApduStatus::Ok;
  }

  std::uint8_t raw = 0;
  status = reader.ReadU8(raw);
  if (status != ApduStatus::Ok) {
    return status;
  }
  value = static_cast<std::int8_t>(raw);
  return ApduStatus::Ok;
}

ApduStatus WriteOptionalInteger8(
  ApduWriter& writer,
  bool present,
  std::int8_t value)
{
  ApduStatus status = WriteAxdrOptionalFlag(writer, present);
  if (status != ApduStatus::Ok || !present) {
    return status;
  }

  return writer.WriteU8(static_cast<std::uint8_t>(value));
}

} // namespace

InitiateRequest MakeDefaultInitiateRequest()
{
  InitiateRequest request;
  request.hasDedicatedKey = false;
  request.dedicatedKey.data = 0;
  request.dedicatedKey.size = 0;
  request.responseAllowed = true;
  request.hasProposedQualityOfService = false;
  request.proposedQualityOfService = 0;
  request.proposedDlmsVersionNumber = 6;
  request.proposedConformance.bytes[0] = 0x00;
  request.proposedConformance.bytes[1] = 0x7e;
  request.proposedConformance.bytes[2] = 0x1f;
  request.clientMaxReceivePduSize = 0x0200;
  return request;
}

ApduStatus DecodeInitiateRequest(
  const std::uint8_t* input,
  std::size_t inputSize,
  InitiateRequest& output)
{
  ApduReader reader(input, inputSize);

  std::uint8_t tag = 0;
  ApduStatus status = reader.ReadU8(tag);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (tag != 0x01) {
    return ApduStatus::InvalidTag;
  }

  bool hasDedicatedKey = false;
  status = ReadAxdrOptionalFlag(reader, hasDedicatedKey);
  if (status != ApduStatus::Ok) {
    return status;
  }
  output.hasDedicatedKey = hasDedicatedKey;
  output.dedicatedKey.data = 0;
  output.dedicatedKey.size = 0;
  if (hasDedicatedKey) {
    AxdrOctetString dedicatedKey = {};
    status = ReadAxdrOctetString(reader, dedicatedKey);
    if (status != ApduStatus::Ok) {
      return status;
    }
    output.dedicatedKey.data = dedicatedKey.data;
    output.dedicatedKey.size = dedicatedKey.size;
  }

  bool hasResponseAllowed = false;
  status = ReadAxdrOptionalFlag(reader, hasResponseAllowed);
  if (status != ApduStatus::Ok) {
    return status;
  }
  output.responseAllowed = true;
  if (hasResponseAllowed) {
    status = ReadAxdrBoolean(reader, output.responseAllowed);
    if (status != ApduStatus::Ok) {
      return status;
    }
  }

  status = ReadOptionalInteger8(
    reader,
    output.hasProposedQualityOfService,
    output.proposedQualityOfService);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = reader.ReadU8(output.proposedDlmsVersionNumber);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = ReadAxdrConformance(reader, output.proposedConformance);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = reader.ReadU16(output.clientMaxReceivePduSize);
  if (status != ApduStatus::Ok) {
    return status;
  }

  return reader.Empty() ? ApduStatus::Ok : ApduStatus::InvalidLength;
}

ApduStatus EncodeInitiateRequest(
  const InitiateRequest& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(0x01);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = WriteAxdrOptionalFlag(writer, input.hasDedicatedKey);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (input.hasDedicatedKey) {
    status = WriteAxdrOctetString(
      writer,
      input.dedicatedKey.data,
      input.dedicatedKey.size);
    if (status != ApduStatus::Ok) {
      return status;
    }
  }

  const bool responseAllowedIsDefault = input.responseAllowed;
  status = WriteAxdrOptionalFlag(writer, !responseAllowedIsDefault);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (!responseAllowedIsDefault) {
    status = WriteAxdrBoolean(writer, input.responseAllowed);
    if (status != ApduStatus::Ok) {
      return status;
    }
  }

  status = WriteOptionalInteger8(
    writer,
    input.hasProposedQualityOfService,
    input.proposedQualityOfService);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = writer.WriteU8(input.proposedDlmsVersionNumber);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = WriteAxdrConformance(writer, input.proposedConformance);
  if (status != ApduStatus::Ok) {
    return status;
  }

  return writer.WriteU16(input.clientMaxReceivePduSize);
}

ApduStatus DecodeInitiateResponse(
  const std::uint8_t* input,
  std::size_t inputSize,
  InitiateResponse& output)
{
  ApduReader reader(input, inputSize);

  std::uint8_t tag = 0;
  ApduStatus status = reader.ReadU8(tag);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (tag != 0x08) {
    return ApduStatus::InvalidTag;
  }

  status = ReadOptionalInteger8(
    reader,
    output.hasNegotiatedQualityOfService,
    output.negotiatedQualityOfService);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = reader.ReadU8(output.negotiatedDlmsVersionNumber);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = ReadAxdrConformance(reader, output.negotiatedConformance);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = reader.ReadU16(output.serverMaxReceivePduSize);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = reader.ReadU16(output.vaaName);
  if (status != ApduStatus::Ok) {
    return status;
  }

  return reader.Empty() ? ApduStatus::Ok : ApduStatus::InvalidLength;
}

ApduStatus EncodeInitiateResponse(
  const InitiateResponse& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU8(0x08);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = WriteOptionalInteger8(
    writer,
    input.hasNegotiatedQualityOfService,
    input.negotiatedQualityOfService);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = writer.WriteU8(input.negotiatedDlmsVersionNumber);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = WriteAxdrConformance(writer, input.negotiatedConformance);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = writer.WriteU16(input.serverMaxReceivePduSize);
  if (status != ApduStatus::Ok) {
    return status;
  }

  return writer.WriteU16(input.vaaName);
}

} // namespace apdu
} // namespace dlms
