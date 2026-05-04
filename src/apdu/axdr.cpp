#include "dlms/apdu/axdr.hpp"

namespace dlms {
namespace apdu {

namespace {

std::size_t AxdrLengthEncodedSize(std::size_t length)
{
  if (length < 0x80) {
    return 1;
  }

  std::size_t encodedSize = 0;
  std::size_t remaining = length;
  while (remaining != 0) {
    remaining >>= 8;
    ++encodedSize;
  }

  return 1 + encodedSize;
}

} // namespace

ApduStatus ReadAxdrBoolean(ApduReader& reader, bool& value)
{
  std::uint8_t raw = 0;
  const ApduStatus status = reader.ReadU8(raw);
  if (status != ApduStatus::Ok) {
    return status;
  }

  value = raw != 0;
  return ApduStatus::Ok;
}

ApduStatus WriteAxdrBoolean(ApduWriter& writer, bool value)
{
  return writer.WriteU8(value ? 0x01 : 0x00);
}

ApduStatus ReadAxdrOptionalFlag(ApduReader& reader, bool& present)
{
  std::uint8_t raw = 0;
  const ApduStatus status = reader.ReadU8(raw);
  if (status != ApduStatus::Ok) {
    return status;
  }

  if (raw > 1) {
    return ApduStatus::InvalidAxdr;
  }

  present = raw != 0;
  return ApduStatus::Ok;
}

ApduStatus WriteAxdrOptionalFlag(ApduWriter& writer, bool present)
{
  return writer.WriteU8(present ? 0x01 : 0x00);
}

ApduStatus ReadAxdrLength(ApduReader& reader, std::size_t& length)
{
  std::uint8_t first = 0;
  ApduStatus status = reader.ReadU8(first);
  if (status != ApduStatus::Ok) {
    return status;
  }

  if ((first & 0x80u) == 0) {
    length = first;
    return ApduStatus::Ok;
  }

  const std::uint8_t lengthOctets = static_cast<std::uint8_t>(first & 0x7fu);
  if (lengthOctets == 0 || lengthOctets > sizeof(std::size_t)) {
    return ApduStatus::InvalidLength;
  }

  std::size_t result = 0;
  for (std::uint8_t i = 0; i < lengthOctets; ++i) {
    std::uint8_t byte = 0;
    status = reader.ReadU8(byte);
    if (status != ApduStatus::Ok) {
      return status;
    }
    result = (result << 8) | byte;
  }

  if (result < 0x80) {
    return ApduStatus::InvalidLength;
  }

  length = result;
  return ApduStatus::Ok;
}

ApduStatus WriteAxdrLength(ApduWriter& writer, std::size_t length)
{
  if (writer.Remaining() < AxdrLengthEncodedSize(length)) {
    return ApduStatus::OutputBufferTooSmall;
  }

  if (length < 0x80) {
    return writer.WriteU8(static_cast<std::uint8_t>(length));
  }

  std::uint8_t encoded[sizeof(std::size_t)] = {};
  std::size_t encodedSize = 0;
  std::size_t remaining = length;
  while (remaining != 0) {
    encoded[sizeof(encoded) - 1 - encodedSize] =
      static_cast<std::uint8_t>(remaining & 0xffu);
    remaining >>= 8;
    ++encodedSize;
  }

  ApduStatus status =
    writer.WriteU8(static_cast<std::uint8_t>(0x80u | encodedSize));
  if (status != ApduStatus::Ok) {
    return status;
  }

  return writer.WriteBytes(encoded + sizeof(encoded) - encodedSize, encodedSize);
}

ApduStatus ReadAxdrOctetString(ApduReader& reader, AxdrOctetString& value)
{
  std::size_t length = 0;
  ApduStatus status = ReadAxdrLength(reader, length);
  if (status != ApduStatus::Ok) {
    return status;
  }

  const std::uint8_t* data = 0;
  status = reader.ReadBytes(data, length);
  if (status != ApduStatus::Ok) {
    return status;
  }

  value.data = data;
  value.size = length;
  return ApduStatus::Ok;
}

ApduStatus WriteAxdrOctetString(
  ApduWriter& writer,
  const std::uint8_t* data,
  std::size_t size)
{
  if (data == 0 && size != 0) {
    return ApduStatus::InvalidArgument;
  }
  if (writer.Remaining() < AxdrLengthEncodedSize(size) + size) {
    return ApduStatus::OutputBufferTooSmall;
  }

  ApduStatus status = WriteAxdrLength(writer, size);
  if (status != ApduStatus::Ok) {
    return status;
  }

  return writer.WriteBytes(data, size);
}

ApduStatus ReadAxdrConformance(ApduReader& reader, AxdrConformance& value)
{
  std::uint8_t tag0 = 0;
  std::uint8_t tag1 = 0;
  std::uint8_t length = 0;
  std::uint8_t unusedBits = 0;

  ApduStatus status = reader.ReadU8(tag0);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = reader.ReadU8(tag1);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = reader.ReadU8(length);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = reader.ReadU8(unusedBits);
  if (status != ApduStatus::Ok) {
    return status;
  }

  if (tag0 != 0x5f || tag1 != 0x1f || length != 0x04 || unusedBits != 0x00) {
    return ApduStatus::InvalidConformance;
  }

  for (std::size_t i = 0; i < 3; ++i) {
    status = reader.ReadU8(value.bytes[i]);
    if (status != ApduStatus::Ok) {
      return status;
    }
  }

  return ApduStatus::Ok;
}

ApduStatus WriteAxdrConformance(
  ApduWriter& writer,
  const AxdrConformance& value)
{
  if (writer.Remaining() < 7) {
    return ApduStatus::OutputBufferTooSmall;
  }

  ApduStatus status = writer.WriteU8(0x5f);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = writer.WriteU8(0x1f);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = writer.WriteU8(0x04);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = writer.WriteU8(0x00);
  if (status != ApduStatus::Ok) {
    return status;
  }

  return writer.WriteBytes(value.bytes, 3);
}

} // namespace apdu
} // namespace dlms
