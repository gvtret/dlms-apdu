#ifndef DLMS_APDU_APDU_TYPES_HPP
#define DLMS_APDU_APDU_TYPES_HPP

#include <cstddef>
#include <cstdint>

namespace dlms {
namespace apdu {

struct ByteView
{
  const std::uint8_t* data;
  std::size_t size;
};

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_APDU_TYPES_HPP
