#include "dlms/apdu/data.hpp"

#include "dlms/apdu/apdu_reader.hpp"
#include "dlms/apdu/axdr.hpp"

namespace dlms {
namespace apdu {

namespace {

constexpr std::uint8_t kNullDataTag = 0x00;
constexpr std::uint8_t kArrayTag = 0x01;
constexpr std::uint8_t kStructureTag = 0x02;
constexpr std::uint8_t kBooleanTag = 0x03;
constexpr std::uint8_t kDoubleLongTag = 0x05;
constexpr std::uint8_t kDoubleLongUnsignedTag = 0x06;
constexpr std::uint8_t kOctetStringTag = 0x09;
constexpr std::uint8_t kIntegerTag = 0x0F;
constexpr std::uint8_t kLongTag = 0x10;
constexpr std::uint8_t kUnsignedTag = 0x11;
constexpr std::uint8_t kLongUnsignedTag = 0x12;
constexpr std::uint8_t kLong64Tag = 0x14;
constexpr std::uint8_t kLong64UnsignedTag = 0x15;
constexpr std::uint8_t kEnumTag = 0x16;

std::int64_t SignExtend(std::uint64_t value, std::size_t byteCount)
{
  const std::uint64_t signBit = 1ULL << ((byteCount * 8U) - 1U);
  if ((value & signBit) == 0U) {
    return static_cast<std::int64_t>(value);
  }
  const std::uint64_t mask = ~0ULL << (byteCount * 8U);
  return static_cast<std::int64_t>(value | mask);
}

ApduStatus ReadUnsigned(ApduReader& reader, std::size_t byteCount, std::uint64_t& value)
{
  value = 0;
  for (std::size_t i = 0; i < byteCount; ++i) {
    std::uint8_t byte = 0;
    ApduStatus status = reader.ReadU8(byte);
    if (status != ApduStatus::Ok) {
      return status;
    }
    value = (value << 8U) | byte;
  }
  return ApduStatus::Ok;
}

ApduStatus WriteUnsigned(ApduWriter& writer, std::uint64_t value, std::size_t byteCount)
{
  for (std::size_t i = 0; i < byteCount; ++i) {
    const std::size_t shift = (byteCount - i - 1U) * 8U;
    ApduStatus status = writer.WriteU8(static_cast<std::uint8_t>(value >> shift));
    if (status != ApduStatus::Ok) {
      return status;
    }
  }
  return ApduStatus::Ok;
}

ApduStatus DecodeDlmsDataValue(
  ApduReader& reader,
  std::size_t maximumDepth,
  DlmsData& output)
{
  if (maximumDepth == 0) {
    return ApduStatus::InvalidData;
  }

  std::uint8_t tag = 0;
  ApduStatus status = reader.ReadU8(tag);
  if (status != ApduStatus::Ok) {
    return status;
  }

  output = {};
  output.type = static_cast<DlmsDataType>(tag);

  std::uint64_t unsignedValue = 0;
  std::size_t elementCount = 0;
  switch (tag) {
    case kNullDataTag:
      return ApduStatus::Ok;

    case kArrayTag:
    case kStructureTag:
      status = ReadAxdrLength(reader, elementCount);
      if (status != ApduStatus::Ok) {
        return status;
      }
      output.elements.reserve(elementCount);
      for (std::size_t i = 0; i < elementCount; ++i) {
        DlmsData element = {};
        status = DecodeDlmsDataValue(reader, maximumDepth - 1U, element);
        if (status != ApduStatus::Ok) {
          return status;
        }
        output.elements.push_back(element);
      }
      return ApduStatus::Ok;

    case kBooleanTag:
      return ReadAxdrBoolean(reader, output.booleanValue);

    case kDoubleLongTag:
      status = ReadUnsigned(reader, 4, unsignedValue);
      output.signedValue = SignExtend(unsignedValue, 4);
      return status;

    case kDoubleLongUnsignedTag:
      status = ReadUnsigned(reader, 4, output.unsignedValue);
      return status;

    case kOctetStringTag:
      {
        AxdrOctetString value = {};
        status = ReadAxdrOctetString(reader, value);
        output.bytes.data = value.data;
        output.bytes.size = value.size;
        return status;
      }

    case kIntegerTag:
      status = ReadUnsigned(reader, 1, unsignedValue);
      output.signedValue = SignExtend(unsignedValue, 1);
      return status;

    case kLongTag:
      status = ReadUnsigned(reader, 2, unsignedValue);
      output.signedValue = SignExtend(unsignedValue, 2);
      return status;

    case kUnsignedTag:
    case kEnumTag:
      status = ReadUnsigned(reader, 1, output.unsignedValue);
      return status;

    case kLongUnsignedTag:
      status = ReadUnsigned(reader, 2, output.unsignedValue);
      return status;

    case kLong64Tag:
      status = ReadUnsigned(reader, 8, unsignedValue);
      output.signedValue = static_cast<std::int64_t>(unsignedValue);
      return status;

    case kLong64UnsignedTag:
      status = ReadUnsigned(reader, 8, output.unsignedValue);
      return status;

    default:
      return ApduStatus::UnsupportedDataType;
  }
}

ApduStatus EncodeDlmsDataValue(const DlmsData& input, ApduWriter& writer)
{
  const std::uint8_t tag = static_cast<std::uint8_t>(input.type);
  ApduStatus status = writer.WriteU8(tag);
  if (status != ApduStatus::Ok) {
    return status;
  }

  switch (tag) {
    case kNullDataTag:
      return ApduStatus::Ok;

    case kArrayTag:
    case kStructureTag:
      status = WriteAxdrLength(writer, input.elements.size());
      if (status != ApduStatus::Ok) {
        return status;
      }
      for (std::size_t i = 0; i < input.elements.size(); ++i) {
        status = EncodeDlmsDataValue(input.elements[i], writer);
        if (status != ApduStatus::Ok) {
          return status;
        }
      }
      return ApduStatus::Ok;

    case kBooleanTag:
      return WriteAxdrBoolean(writer, input.booleanValue);

    case kDoubleLongTag:
      return WriteUnsigned(writer, static_cast<std::uint32_t>(input.signedValue), 4);

    case kDoubleLongUnsignedTag:
      return WriteUnsigned(writer, input.unsignedValue, 4);

    case kOctetStringTag:
      return WriteAxdrOctetString(writer, input.bytes.data, input.bytes.size);

    case kIntegerTag:
      return WriteUnsigned(writer, static_cast<std::uint8_t>(input.signedValue), 1);

    case kLongTag:
      return WriteUnsigned(writer, static_cast<std::uint16_t>(input.signedValue), 2);

    case kUnsignedTag:
    case kEnumTag:
      return WriteUnsigned(writer, input.unsignedValue, 1);

    case kLongUnsignedTag:
      return WriteUnsigned(writer, input.unsignedValue, 2);

    case kLong64Tag:
      return WriteUnsigned(writer, static_cast<std::uint64_t>(input.signedValue), 8);

    case kLong64UnsignedTag:
      return WriteUnsigned(writer, input.unsignedValue, 8);

    default:
      return ApduStatus::UnsupportedDataType;
  }
}

} // namespace

ApduStatus DecodeDlmsData(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDepth,
  DlmsData& output)
{
  if (input == nullptr && inputSize != 0) {
    return ApduStatus::InvalidArgument;
  }

  ApduReader reader(input, inputSize);
  ApduStatus status = DecodeDlmsDataFromReader(reader, maximumDepth, output);
  if (status != ApduStatus::Ok) {
    return status;
  }
  if (!reader.Empty()) {
    return ApduStatus::InvalidLength;
  }
  return ApduStatus::Ok;
}

ApduStatus DecodeDlmsDataFromReader(
  ApduReader& reader,
  std::size_t maximumDepth,
  DlmsData& output)
{
  return DecodeDlmsDataValue(reader, maximumDepth, output);
}

ApduStatus EncodeDlmsData(
  const DlmsData& input,
  ApduWriter& writer)
{
  return EncodeDlmsDataValue(input, writer);
}

} // namespace apdu
} // namespace dlms
