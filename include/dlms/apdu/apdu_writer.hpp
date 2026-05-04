#ifndef DLMS_APDU_APDU_WRITER_HPP
#define DLMS_APDU_APDU_WRITER_HPP

#include "dlms/apdu/apdu_error.hpp"

#include <cstddef>
#include <cstdint>

namespace dlms {
namespace apdu {

class ApduWriter
{
public:
  ApduWriter(std::uint8_t* data, std::size_t size);

  std::size_t Remaining() const;
  std::size_t WrittenSize() const;

  ApduStatus WriteU8(std::uint8_t value);
  ApduStatus WriteU16(std::uint16_t value);
  ApduStatus WriteU32(std::uint32_t value);
  ApduStatus WriteBytes(const std::uint8_t* data, std::size_t size);

private:
  std::uint8_t* data_;
  std::size_t size_;
  std::size_t position_;
};

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_APDU_WRITER_HPP
