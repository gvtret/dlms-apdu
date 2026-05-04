#include "dlms/apdu/ber.hpp"

#include <limits>

namespace dlms {
namespace apdu {

namespace {

std::size_t BerLengthEncodedSize(std::size_t length)
{
  if (length < 128) {
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

ApduStatus ValidateBerReader(ApduReader& reader, std::size_t maximumDepth)
{
  while (!reader.Empty()) {
    BerTlv tlv;
    const ApduStatus status = ReadBerTlv(reader, tlv);
    if (status != ApduStatus::Ok) {
      return status;
    }

    if (BerTagIsConstructed(tlv.tag)) {
      if (maximumDepth == 0) {
        return ApduStatus::InvalidBer;
      }

      const ApduStatus childStatus =
        ValidateBerTlv(tlv.value, tlv.valueSize, maximumDepth - 1);
      if (childStatus != ApduStatus::Ok) {
        return childStatus;
      }
    }
  }

  return ApduStatus::Ok;
}

} // namespace

bool BerTagIsConstructed(std::uint8_t tag)
{
  return (tag & 0x20u) != 0;
}

ApduStatus ReadBerLength(ApduReader& reader, std::size_t& length)
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
  if (lengthOctets == 0) {
    return ApduStatus::UnsupportedFeature;
  }
  if (lengthOctets > sizeof(std::size_t)) {
    return ApduStatus::InvalidLength;
  }

  std::size_t result = 0;
  for (std::uint8_t i = 0; i < lengthOctets; ++i) {
    std::uint8_t byte = 0;
    status = reader.ReadU8(byte);
    if (status != ApduStatus::Ok) {
      return status;
    }

    if (result > (std::numeric_limits<std::size_t>::max() >> 8)) {
      return ApduStatus::InvalidLength;
    }
    result = (result << 8) | byte;
  }

  if (result < 128) {
    return ApduStatus::InvalidLength;
  }

  length = result;
  return ApduStatus::Ok;
}

ApduStatus WriteBerLength(ApduWriter& writer, std::size_t length)
{
  if (writer.Remaining() < BerLengthEncodedSize(length)) {
    return ApduStatus::OutputBufferTooSmall;
  }

  if (length < 128) {
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

ApduStatus ReadBerTlv(ApduReader& reader, BerTlv& tlv)
{
  const std::size_t startPosition = reader.Position();

  std::uint8_t tag = 0;
  ApduStatus status = reader.ReadU8(tag);
  if (status != ApduStatus::Ok) {
    return status;
  }

  if ((tag & 0x1fu) == 0x1fu) {
    return ApduStatus::UnsupportedFeature;
  }

  std::size_t length = 0;
  status = ReadBerLength(reader, length);
  if (status != ApduStatus::Ok) {
    return status;
  }

  const std::uint8_t* value = 0;
  status = reader.ReadBytes(value, length);
  if (status != ApduStatus::Ok) {
    return status;
  }

  tlv.tag = tag;
  tlv.value = value;
  tlv.valueSize = length;
  tlv.headerSize = reader.Position() - startPosition - length;
  return ApduStatus::Ok;
}

ApduStatus WriteBerTlv(
  ApduWriter& writer,
  std::uint8_t tag,
  const std::uint8_t* value,
  std::size_t valueSize)
{
  if ((tag & 0x1fu) == 0x1fu) {
    return ApduStatus::UnsupportedFeature;
  }
  if (value == 0 && valueSize != 0) {
    return ApduStatus::InvalidArgument;
  }
  if (writer.Remaining() < 1 + BerLengthEncodedSize(valueSize) + valueSize) {
    return ApduStatus::OutputBufferTooSmall;
  }

  ApduStatus status = writer.WriteU8(tag);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = WriteBerLength(writer, valueSize);
  if (status != ApduStatus::Ok) {
    return status;
  }

  return writer.WriteBytes(value, valueSize);
}

ApduStatus ValidateBerTlv(
  const std::uint8_t* data,
  std::size_t size,
  std::size_t maximumDepth)
{
  if (data == 0 && size != 0) {
    return ApduStatus::InvalidArgument;
  }

  ApduReader reader(data, size);
  return ValidateBerReader(reader, maximumDepth);
}

} // namespace apdu
} // namespace dlms
