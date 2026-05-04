#ifndef DLMS_APDU_BER_HPP
#define DLMS_APDU_BER_HPP

#include "dlms/apdu/apdu_error.hpp"
#include "dlms/apdu/apdu_reader.hpp"
#include "dlms/apdu/apdu_writer.hpp"

#include <cstddef>
#include <cstdint>

namespace dlms {
namespace apdu {

struct BerTlv
{
  std::uint8_t tag;
  const std::uint8_t* value;
  std::size_t valueSize;
  std::size_t headerSize;
};

bool BerTagIsConstructed(std::uint8_t tag);

ApduStatus ReadBerLength(ApduReader& reader, std::size_t& length);

ApduStatus WriteBerLength(ApduWriter& writer, std::size_t length);

ApduStatus ReadBerTlv(ApduReader& reader, BerTlv& tlv);

ApduStatus WriteBerTlv(
  ApduWriter& writer,
  std::uint8_t tag,
  const std::uint8_t* value,
  std::size_t valueSize);

ApduStatus ValidateBerTlv(
  const std::uint8_t* data,
  std::size_t size,
  std::size_t maximumDepth);

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_BER_HPP
