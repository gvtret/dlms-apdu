#ifndef DLMS_APDU_DATA_HPP
#define DLMS_APDU_DATA_HPP

#include "dlms/apdu/apdu_error.hpp"
#include "dlms/apdu/apdu_reader.hpp"
#include "dlms/apdu/apdu_types.hpp"
#include "dlms/apdu/apdu_writer.hpp"

#include <cstdint>
#include <vector>

namespace dlms {
namespace apdu {

enum class DlmsDataType : std::uint8_t
{
  NullData = 0,
  Array = 1,
  Structure = 2,
  Boolean = 3,
  DoubleLong = 5,
  DoubleLongUnsigned = 6,
  OctetString = 9,
  Integer = 15,
  Long = 16,
  Unsigned = 17,
  LongUnsigned = 18,
  Long64 = 20,
  Long64Unsigned = 21,
  Enum = 22
};

struct DlmsData
{
  DlmsDataType type;
  bool booleanValue;
  std::int64_t signedValue;
  std::uint64_t unsignedValue;
  ByteView bytes;
  std::vector<DlmsData> elements;
};

ApduStatus DecodeDlmsData(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDepth,
  DlmsData& output);

ApduStatus DecodeDlmsDataFromReader(
  ApduReader& reader,
  std::size_t maximumDepth,
  DlmsData& output);

ApduStatus EncodeDlmsData(
  const DlmsData& input,
  ApduWriter& writer);

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_DATA_HPP
