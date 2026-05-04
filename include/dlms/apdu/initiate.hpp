#ifndef DLMS_APDU_INITIATE_HPP
#define DLMS_APDU_INITIATE_HPP

#include "dlms/apdu/apdu_error.hpp"
#include "dlms/apdu/apdu_reader.hpp"
#include "dlms/apdu/apdu_types.hpp"
#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/axdr.hpp"

#include <cstdint>

namespace dlms {
namespace apdu {

struct InitiateRequest
{
  bool hasDedicatedKey;
  ByteView dedicatedKey;
  bool responseAllowed;
  bool hasProposedQualityOfService;
  std::int8_t proposedQualityOfService;
  std::uint8_t proposedDlmsVersionNumber;
  AxdrConformance proposedConformance;
  std::uint16_t clientMaxReceivePduSize;
};

struct InitiateResponse
{
  bool hasNegotiatedQualityOfService;
  std::int8_t negotiatedQualityOfService;
  std::uint8_t negotiatedDlmsVersionNumber;
  AxdrConformance negotiatedConformance;
  std::uint16_t serverMaxReceivePduSize;
  std::uint16_t vaaName;
};

InitiateRequest MakeDefaultInitiateRequest();

ApduStatus DecodeInitiateRequest(
  const std::uint8_t* input,
  std::size_t inputSize,
  InitiateRequest& output);

ApduStatus EncodeInitiateRequest(
  const InitiateRequest& input,
  ApduWriter& writer);

ApduStatus DecodeInitiateResponse(
  const std::uint8_t* input,
  std::size_t inputSize,
  InitiateResponse& output);

ApduStatus EncodeInitiateResponse(
  const InitiateResponse& input,
  ApduWriter& writer);

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_INITIATE_HPP
