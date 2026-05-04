#ifndef DLMS_APDU_SET_HPP
#define DLMS_APDU_SET_HPP

#include "dlms/apdu/apdu_error.hpp"
#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/cosem_descriptor.hpp"
#include "dlms/apdu/data.hpp"
#include "dlms/apdu/get.hpp"

#include <cstdint>
#include <vector>

namespace dlms {
namespace apdu {

struct SetRequestNormal
{
  std::uint8_t invokeIdAndPriority;
  CosemAttributeDescriptor descriptor;
  bool hasSelectiveAccess;
  SelectiveAccessDescriptor selectiveAccess;
  DlmsData data;
};

struct SetResponseNormal
{
  std::uint8_t invokeIdAndPriority;
  std::uint8_t result;
};

struct DataBlockSA
{
  bool lastBlock;
  std::uint32_t blockNumber;
  ByteView rawData;
};

enum class SetRequestChoice : std::uint8_t
{
  Normal = 1,
  WithFirstDataBlock = 2,
  WithDataBlock = 3,
  WithList = 4,
  WithListAndFirstDataBlock = 5
};

struct SetRequest
{
  SetRequestChoice choice;
  std::uint8_t invokeIdAndPriority;
  CosemAttributeDescriptorWithSelection normal;
  DlmsData data;
  DataBlockSA dataBlock;
  std::vector<CosemAttributeDescriptorWithSelection> list;
  std::vector<DlmsData> valueList;
};

enum class SetResponseChoice : std::uint8_t
{
  Normal = 1,
  DataBlock = 2,
  LastDataBlock = 3,
  LastDataBlockWithList = 4,
  WithList = 5
};

struct SetResponse
{
  SetResponseChoice choice;
  std::uint8_t invokeIdAndPriority;
  std::uint8_t result;
  std::uint32_t blockNumber;
  std::vector<std::uint8_t> resultList;
};

ApduStatus DecodeSetRequestNormal(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  SetRequestNormal& output);

ApduStatus EncodeSetRequestNormal(
  const SetRequestNormal& input,
  ApduWriter& writer);

ApduStatus DecodeSetResponseNormal(
  const std::uint8_t* input,
  std::size_t inputSize,
  SetResponseNormal& output);

ApduStatus EncodeSetResponseNormal(
  const SetResponseNormal& input,
  ApduWriter& writer);

ApduStatus DecodeSetRequest(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  SetRequest& output);

ApduStatus EncodeSetRequest(
  const SetRequest& input,
  ApduWriter& writer);

ApduStatus DecodeSetResponse(
  const std::uint8_t* input,
  std::size_t inputSize,
  SetResponse& output);

ApduStatus EncodeSetResponse(
  const SetResponse& input,
  ApduWriter& writer);

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_SET_HPP
