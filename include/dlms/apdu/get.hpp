#ifndef DLMS_APDU_GET_HPP
#define DLMS_APDU_GET_HPP

#include "dlms/apdu/apdu_error.hpp"
#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/cosem_descriptor.hpp"
#include "dlms/apdu/data.hpp"

#include <cstdint>
#include <vector>

namespace dlms {
namespace apdu {

enum class GetDataResultChoice : std::uint8_t
{
  Data = 0,
  DataAccessError = 1
};

struct SelectiveAccessDescriptor
{
  std::uint8_t selector;
  DlmsData parameters;
};

struct CosemAttributeDescriptorWithSelection
{
  CosemAttributeDescriptor descriptor;
  bool hasSelection;
  SelectiveAccessDescriptor selection;
};

struct DataBlockG
{
  bool lastBlock;
  std::uint32_t blockNumber;
  ByteView rawData;
};

struct GetDataResult
{
  GetDataResultChoice choice;
  DlmsData data;
  std::uint8_t dataAccessError;
};

struct GetRequestNormal
{
  std::uint8_t invokeIdAndPriority;
  CosemAttributeDescriptor descriptor;
  bool hasSelectiveAccess;
  SelectiveAccessDescriptor selectiveAccess;
};

struct GetResponseNormal
{
  std::uint8_t invokeIdAndPriority;
  GetDataResultChoice resultChoice;
  DlmsData data;
  std::uint8_t dataAccessError;
};

enum class GetRequestChoice : std::uint8_t
{
  Normal = 1,
  Next = 2,
  WithList = 3
};

struct GetRequest
{
  GetRequestChoice choice;
  std::uint8_t invokeIdAndPriority;
  CosemAttributeDescriptorWithSelection normal;
  std::uint32_t blockNumber;
  std::vector<CosemAttributeDescriptorWithSelection> list;
};

enum class GetResponseChoice : std::uint8_t
{
  Normal = 1,
  WithDataBlock = 2,
  WithList = 3
};

struct GetResponse
{
  GetResponseChoice choice;
  std::uint8_t invokeIdAndPriority;
  GetDataResult result;
  DataBlockG dataBlock;
  std::vector<GetDataResult> list;
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

ApduStatus DecodeGetRequest(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  GetRequest& output);

ApduStatus EncodeGetRequest(
  const GetRequest& input,
  ApduWriter& writer);

ApduStatus DecodeGetResponse(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  GetResponse& output);

ApduStatus EncodeGetResponse(
  const GetResponse& input,
  ApduWriter& writer);

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_GET_HPP
