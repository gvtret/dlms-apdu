#include "dlms/apdu/cosem_descriptor.hpp"

namespace dlms {
namespace apdu {

ApduStatus DecodeCosemAttributeDescriptor(
  ApduReader& reader,
  CosemAttributeDescriptor& output)
{
  ApduStatus status = reader.ReadU16(output.classId);
  if (status != ApduStatus::Ok) {
    return status;
  }
  for (std::size_t i = 0; i < 6; ++i) {
    status = reader.ReadU8(output.logicalName[i]);
    if (status != ApduStatus::Ok) {
      return status;
    }
  }
  return reader.ReadU8(output.attributeId);
}

ApduStatus EncodeCosemAttributeDescriptor(
  const CosemAttributeDescriptor& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU16(input.classId);
  if (status != ApduStatus::Ok) {
    return status;
  }
  for (std::size_t i = 0; i < 6; ++i) {
    status = writer.WriteU8(input.logicalName[i]);
    if (status != ApduStatus::Ok) {
      return status;
    }
  }
  return writer.WriteU8(input.attributeId);
}

ApduStatus DecodeCosemMethodDescriptor(
  ApduReader& reader,
  CosemMethodDescriptor& output)
{
  ApduStatus status = reader.ReadU16(output.classId);
  if (status != ApduStatus::Ok) {
    return status;
  }
  for (std::size_t i = 0; i < 6; ++i) {
    status = reader.ReadU8(output.logicalName[i]);
    if (status != ApduStatus::Ok) {
      return status;
    }
  }
  return reader.ReadU8(output.methodId);
}

ApduStatus EncodeCosemMethodDescriptor(
  const CosemMethodDescriptor& input,
  ApduWriter& writer)
{
  ApduStatus status = writer.WriteU16(input.classId);
  if (status != ApduStatus::Ok) {
    return status;
  }
  for (std::size_t i = 0; i < 6; ++i) {
    status = writer.WriteU8(input.logicalName[i]);
    if (status != ApduStatus::Ok) {
      return status;
    }
  }
  return writer.WriteU8(input.methodId);
}

} // namespace apdu
} // namespace dlms
