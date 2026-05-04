#ifndef DLMS_APDU_GET_HPP
#define DLMS_APDU_GET_HPP

#include "dlms/apdu/apdu_error.hpp"
#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/cosem_descriptor.hpp"
#include "dlms/apdu/data.hpp"

#include <cstdint>

namespace dlms {
namespace apdu {

enum class GetDataResultChoice : std::uint8_t
{
  Data = 0,
  DataAccessError = 1
};

struct GetRequestNormal
{
  std::uint8_t invokeIdAndPriority;
  CosemAttributeDescriptor descriptor;
  bool hasSelectiveAccess;
};

struct GetResponseNormal
{
  std::uint8_t invokeIdAndPriority;
  GetDataResultChoice resultChoice;
  DlmsData data;
  std::uint8_t dataAccessError;
};

ApduStatus DecodeGetRequestNormal(
  const std::uint8_t* input,
  std::size_t inputSize,
  GetRequestNormal& output);

ApduStatus EncodeGetRequestNormal(
  const GetRequestNormal& input,
  ApduWriter& writer);

ApduStatus DecodeGetResponseNormal(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  GetResponseNormal& output);

ApduStatus EncodeGetResponseNormal(
  const GetResponseNormal& input,
  ApduWriter& writer);

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_GET_HPP
