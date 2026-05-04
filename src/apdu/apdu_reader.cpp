#include "dlms/apdu/apdu_reader.hpp"

namespace dlms {
namespace apdu {

ApduReader::ApduReader(const std::uint8_t* data, std::size_t size)
  : data_(data),
    size_(size),
    position_(0)
{
}

std::size_t ApduReader::Remaining() const
{
  return size_ - position_;
}

std::size_t ApduReader::Position() const
{
  return position_;
}

bool ApduReader::Empty() const
{
  return Remaining() == 0;
}

ApduStatus ApduReader::ReadU8(std::uint8_t& value)
{
  if (data_ == 0 && size_ != 0) {
    return ApduStatus::InvalidArgument;
  }
  if (Remaining() < 1) {
    return ApduStatus::NeedMoreData;
  }

  value = data_[position_];
  ++position_;
  return ApduStatus::Ok;
}

ApduStatus ApduReader::ReadU16(std::uint16_t& value)
{
  if (data_ == 0 && size_ != 0) {
    return ApduStatus::InvalidArgument;
  }
  if (Remaining() < 2) {
    return ApduStatus::NeedMoreData;
  }

  std::uint8_t high = 0;
  std::uint8_t low = 0;
  ApduStatus status = ReadU8(high);
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = ReadU8(low);
  if (status != ApduStatus::Ok) {
    return status;
  }

  value = static_cast<std::uint16_t>(
    (static_cast<std::uint16_t>(high) << 8) | low);
  return ApduStatus::Ok;
}

ApduStatus ApduReader::ReadU32(std::uint32_t& value)
{
  if (data_ == 0 && size_ != 0) {
    return ApduStatus::InvalidArgument;
  }
  if (Remaining() < 4) {
    return ApduStatus::NeedMoreData;
  }

  std::uint8_t b0 = 0;
  std::uint8_t b1 = 0;
  std::uint8_t b2 = 0;
  std::uint8_t b3 = 0;

  ApduStatus status = ReadU8(b0);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = ReadU8(b1);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = ReadU8(b2);
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = ReadU8(b3);
  if (status != ApduStatus::Ok) {
    return status;
  }

  value = (static_cast<std::uint32_t>(b0) << 24) |
          (static_cast<std::uint32_t>(b1) << 16) |
          (static_cast<std::uint32_t>(b2) << 8) |
          b3;
  return ApduStatus::Ok;
}

ApduStatus ApduReader::ReadBytes(
  const std::uint8_t*& data,
  std::size_t size)
{
  if (data_ == 0 && size_ != 0) {
    return ApduStatus::InvalidArgument;
  }
  if (Remaining() < size) {
    return ApduStatus::NeedMoreData;
  }

  data = data_ + position_;
  position_ += size;
  return ApduStatus::Ok;
}

ApduStatus ApduReader::Skip(std::size_t size)
{
  const std::uint8_t* ignored = 0;
  return ReadBytes(ignored, size);
}

} // namespace apdu
} // namespace dlms
