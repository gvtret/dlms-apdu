#ifndef DLMS_APDU_APDU_READER_HPP
#define DLMS_APDU_APDU_READER_HPP

#include "dlms/apdu/apdu_error.hpp"

#include <cstddef>
#include <cstdint>

namespace dlms {
namespace apdu {

class ApduReader
{
public:
  ApduReader(const std::uint8_t* data, std::size_t size);

  std::size_t Remaining() const;
  std::size_t Position() const;
  bool Empty() const;

  ApduStatus ReadU8(std::uint8_t& value);
  ApduStatus ReadU16(std::uint16_t& value);
  ApduStatus ReadU32(std::uint32_t& value);
  ApduStatus ReadBytes(const std::uint8_t*& data, std::size_t size);
  ApduStatus Skip(std::size_t size);

private:
  const std::uint8_t* data_;
  std::size_t size_;
  std::size_t position_;
};

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_APDU_READER_HPP
