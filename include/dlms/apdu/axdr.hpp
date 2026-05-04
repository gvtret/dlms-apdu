#ifndef DLMS_APDU_AXDR_HPP
#define DLMS_APDU_AXDR_HPP

#include "dlms/apdu/apdu_error.hpp"
#include "dlms/apdu/apdu_reader.hpp"
#include "dlms/apdu/apdu_writer.hpp"

#include <cstddef>
#include <cstdint>

namespace dlms {
namespace apdu {

struct AxdrOctetString
{
  const std::uint8_t* data;
  std::size_t size;
};

struct AxdrConformance
{
  std::uint8_t bytes[3];
};

ApduStatus ReadAxdrBoolean(ApduReader& reader, bool& value);
ApduStatus WriteAxdrBoolean(ApduWriter& writer, bool value);

ApduStatus ReadAxdrOptionalFlag(ApduReader& reader, bool& present);
ApduStatus WriteAxdrOptionalFlag(ApduWriter& writer, bool present);

ApduStatus ReadAxdrLength(ApduReader& reader, std::size_t& length);
ApduStatus WriteAxdrLength(ApduWriter& writer, std::size_t length);

ApduStatus ReadAxdrOctetString(ApduReader& reader, AxdrOctetString& value);
ApduStatus WriteAxdrOctetString(
  ApduWriter& writer,
  const std::uint8_t* data,
  std::size_t size);

ApduStatus ReadAxdrConformance(ApduReader& reader, AxdrConformance& value);
ApduStatus WriteAxdrConformance(
  ApduWriter& writer,
  const AxdrConformance& value);

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_AXDR_HPP
