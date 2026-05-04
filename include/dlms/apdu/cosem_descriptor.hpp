#ifndef DLMS_APDU_COSEM_DESCRIPTOR_HPP
#define DLMS_APDU_COSEM_DESCRIPTOR_HPP

#include "dlms/apdu/apdu_error.hpp"
#include "dlms/apdu/apdu_reader.hpp"
#include "dlms/apdu/apdu_writer.hpp"

#include <cstdint>

namespace dlms {
namespace apdu {

struct CosemAttributeDescriptor
{
  std::uint16_t classId;
  std::uint8_t logicalName[6];
  std::uint8_t attributeId;
};

struct CosemMethodDescriptor
{
  std::uint16_t classId;
  std::uint8_t logicalName[6];
  std::uint8_t methodId;
};

ApduStatus DecodeCosemAttributeDescriptor(
  ApduReader& reader,
  CosemAttributeDescriptor& output);

ApduStatus EncodeCosemAttributeDescriptor(
  const CosemAttributeDescriptor& input,
  ApduWriter& writer);

ApduStatus DecodeCosemMethodDescriptor(
  ApduReader& reader,
  CosemMethodDescriptor& output);

ApduStatus EncodeCosemMethodDescriptor(
  const CosemMethodDescriptor& input,
  ApduWriter& writer);

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_COSEM_DESCRIPTOR_HPP
