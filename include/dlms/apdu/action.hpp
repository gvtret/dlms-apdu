#ifndef DLMS_APDU_ACTION_HPP
#define DLMS_APDU_ACTION_HPP

#include "dlms/apdu/apdu_error.hpp"
#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/cosem_descriptor.hpp"
#include "dlms/apdu/data.hpp"
#include "dlms/apdu/set.hpp"

#include <cstdint>
#include <vector>

namespace dlms {
namespace apdu {

struct ActionRequestNormal
{
  std::uint8_t invokeIdAndPriority;
  CosemMethodDescriptor descriptor;
  bool hasInvocationParameter;
  DlmsData invocationParameter;
};

struct ActionResponseNormal
{
  std::uint8_t invokeIdAndPriority;
  std::uint8_t result;
  bool hasReturnParameter;
  DlmsData returnParameter;
};

struct ActionResponseItem
{
  std::uint8_t result;
  bool hasReturnParameter;
  DlmsData returnParameter;
};

struct CosemMethodDescriptorWithParameter
{
  CosemMethodDescriptor descriptor;
  bool hasInvocationParameter;
  DlmsData invocationParameter;
};

enum class ActionRequestChoice : std::uint8_t
{
  Normal = 1,
  NextPblock = 2,
  WithList = 3,
  WithFirstPblock = 4,
  WithListAndFirstPblock = 5,
  WithPblock = 6
};

struct ActionRequest
{
  ActionRequestChoice choice;
  std::uint8_t invokeIdAndPriority;
  CosemMethodDescriptorWithParameter normal;
  std::uint32_t blockNumber;
  DataBlockSA dataBlock;
  std::vector<CosemMethodDescriptorWithParameter> list;
  std::vector<CosemMethodDescriptor> descriptorList;
};

enum class ActionResponseChoice : std::uint8_t
{
  Normal = 1,
  WithPblock = 2,
  WithList = 3,
  NextPblock = 4
};

struct ActionResponse
{
  ActionResponseChoice choice;
  std::uint8_t invokeIdAndPriority;
  ActionResponseItem normal;
  DataBlockG dataBlock;
  std::vector<ActionResponseItem> list;
  std::uint32_t blockNumber;
};

ApduStatus DecodeActionRequestNormal(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  ActionRequestNormal& output);

ApduStatus EncodeActionRequestNormal(
  const ActionRequestNormal& input,
  ApduWriter& writer);

ApduStatus DecodeActionResponseNormal(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  ActionResponseNormal& output);

ApduStatus EncodeActionResponseNormal(
  const ActionResponseNormal& input,
  ApduWriter& writer);

ApduStatus DecodeActionRequest(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  ActionRequest& output);

ApduStatus EncodeActionRequest(
  const ActionRequest& input,
  ApduWriter& writer);

ApduStatus DecodeActionResponse(
  const std::uint8_t* input,
  std::size_t inputSize,
  std::size_t maximumDataDepth,
  ActionResponse& output);

ApduStatus EncodeActionResponse(
  const ActionResponse& input,
  ApduWriter& writer);

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_ACTION_HPP
