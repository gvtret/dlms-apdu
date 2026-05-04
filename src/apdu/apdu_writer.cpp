#include "dlms/apdu/apdu_writer.hpp"

namespace dlms {
namespace apdu {

ApduWriter::ApduWriter(std::uint8_t* data, std::size_t size)
  : data_(data),
    size_(size),
    position_(0)
{
}

std::size_t ApduWriter::Remaining() const
{
  return size_ - position_;
}

std::size_t ApduWriter::WrittenSize() const
{
  return position_;
}

ApduStatus ApduWriter::WriteU8(std::uint8_t value)
{
  if (data_ == 0 && size_ != 0) {
    return ApduStatus::InvalidArgument;
  }
  if (Remaining() < 1) {
    return ApduStatus::OutputBufferTooSmall;
  }

  data_[position_] = value;
  ++position_;
  return ApduStatus::Ok;
}

ApduStatus ApduWriter::WriteU16(std::uint16_t value)
{
  if (data_ == 0 && size_ != 0) {
    return ApduStatus::InvalidArgument;
  }
  if (Remaining() < 2) {
    return ApduStatus::OutputBufferTooSmall;
  }

  ApduStatus status = WriteU8(static_cast<std::uint8_t>((value >> 8) & 0xff));
  if (status != ApduStatus::Ok) {
    return status;
  }

  status = WriteU8(static_cast<std::uint8_t>(value & 0xff));
  if (status != ApduStatus::Ok) {
    return status;
  }

  return ApduStatus::Ok;
}

ApduStatus ApduWriter::WriteU32(std::uint32_t value)
{
  if (data_ == 0 && size_ != 0) {
    return ApduStatus::InvalidArgument;
  }
  if (Remaining() < 4) {
    return ApduStatus::OutputBufferTooSmall;
  }

  ApduStatus status = WriteU8(static_cast<std::uint8_t>((value >> 24) & 0xff));
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = WriteU8(static_cast<std::uint8_t>((value >> 16) & 0xff));
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = WriteU8(static_cast<std::uint8_t>((value >> 8) & 0xff));
  if (status != ApduStatus::Ok) {
    return status;
  }
  status = WriteU8(static_cast<std::uint8_t>(value & 0xff));
  if (status != ApduStatus::Ok) {
    return status;
  }

  return ApduStatus::Ok;
}

ApduStatus ApduWriter::WriteBytes(
  const std::uint8_t* data,
  std::size_t size)
{
  if (data_ == 0 && size_ != 0) {
    return ApduStatus::InvalidArgument;
  }
  if (data == 0 && size != 0) {
    return ApduStatus::InvalidArgument;
  }
  if (Remaining() < size) {
    return ApduStatus::OutputBufferTooSmall;
  }

  for (std::size_t i = 0; i < size; ++i) {
    data_[position_ + i] = data[i];
  }
  position_ += size;
  return ApduStatus::Ok;
}

} // namespace apdu
} // namespace dlms
