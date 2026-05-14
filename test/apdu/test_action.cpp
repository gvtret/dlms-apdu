#include "dlms/apdu/action.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>

namespace {

using dlms::apdu::ActionRequestNormal;
using dlms::apdu::ActionRequest;
using dlms::apdu::ActionRequestChoice;
using dlms::apdu::ActionResponse;
using dlms::apdu::ActionResponseChoice;
using dlms::apdu::ActionResponseNormal;
using dlms::apdu::ApduStatus;
using dlms::apdu::ApduWriter;
using dlms::apdu::DecodeActionRequest;
using dlms::apdu::DecodeActionRequestNormal;
using dlms::apdu::DecodeActionResponse;
using dlms::apdu::DecodeActionResponseNormal;
using dlms::apdu::DlmsDataType;
using dlms::apdu::EncodeActionRequest;
using dlms::apdu::EncodeActionRequestNormal;
using dlms::apdu::EncodeActionResponse;
using dlms::apdu::EncodeActionResponseNormal;

constexpr std::array<std::uint8_t, 15> kActionRequestNormal = {
  0xC3, 0x01, 0x81, 0x00, 0x0F, 0x00, 0x00, 0x28,
  0x00, 0x00, 0xFF, 0x01, 0x01, 0x11, 0x01};

constexpr std::array<std::uint8_t, 8> kActionResponseNormal = {
  0xC7, 0x01, 0x81, 0x00, 0x01, 0x00, 0x11, 0x2A};

} // namespace

TEST(ActionCodecTest, DecodesActionRequestNormal)
{
  ActionRequestNormal request = {};

  EXPECT_EQ(
    DecodeActionRequestNormal(
      kActionRequestNormal.data(),
      kActionRequestNormal.size(),
      4,
      request),
    ApduStatus::Ok);
  EXPECT_EQ(request.invokeIdAndPriority, 0x81);
  EXPECT_EQ(request.descriptor.classId, 0x000FU);
  EXPECT_EQ(request.descriptor.logicalName[2], 0x28);
  EXPECT_EQ(request.descriptor.logicalName[5], 0xFF);
  EXPECT_EQ(request.descriptor.methodId, 0x01);
  EXPECT_TRUE(request.hasInvocationParameter);
  EXPECT_EQ(request.invocationParameter.type, DlmsDataType::Unsigned);
  EXPECT_EQ(request.invocationParameter.unsignedValue, 0x01U);
}

TEST(ActionCodecTest, EncodesActionRequestNormal)
{
  ActionRequestNormal request = {};
  ASSERT_EQ(
    DecodeActionRequestNormal(
      kActionRequestNormal.data(),
      kActionRequestNormal.size(),
      4,
      request),
    ApduStatus::Ok);

  std::array<std::uint8_t, 32> output = {};
  ApduWriter writer(output.data(), output.size());

  EXPECT_EQ(EncodeActionRequestNormal(request, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), kActionRequestNormal.size());
  EXPECT_EQ(std::equal(kActionRequestNormal.begin(), kActionRequestNormal.end(), output.begin()), true);
}

TEST(ActionCodecTest, DecodesActionResponseNormal)
{
  ActionResponseNormal response = {};

  EXPECT_EQ(
    DecodeActionResponseNormal(
      kActionResponseNormal.data(),
      kActionResponseNormal.size(),
      4,
      response),
    ApduStatus::Ok);
  EXPECT_EQ(response.invokeIdAndPriority, 0x81);
  EXPECT_EQ(response.result, 0x00);
  EXPECT_TRUE(response.hasReturnParameter);
  EXPECT_EQ(response.returnParameter.type, DlmsDataType::Unsigned);
  EXPECT_EQ(response.returnParameter.unsignedValue, 0x2AU);
}

TEST(ActionCodecTest, EncodesActionResponseNormal)
{
  ActionResponseNormal response = {};
  ASSERT_EQ(
    DecodeActionResponseNormal(
      kActionResponseNormal.data(),
      kActionResponseNormal.size(),
      4,
      response),
    ApduStatus::Ok);

  std::array<std::uint8_t, 16> output = {};
  ApduWriter writer(output.data(), output.size());

  EXPECT_EQ(EncodeActionResponseNormal(response, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), kActionResponseNormal.size());
  EXPECT_EQ(std::equal(kActionResponseNormal.begin(), kActionResponseNormal.end(), output.begin()), true);
}

TEST(ActionCodecTest, DecodesActionWithoutOptionalData)
{
  constexpr std::array<std::uint8_t, 13> input = {
    0xC3, 0x01, 0x81, 0x00, 0x0F, 0x00, 0x00,
    0x28, 0x00, 0x00, 0xFF, 0x01, 0x00};
  ActionRequestNormal request = {};

  EXPECT_EQ(
    DecodeActionRequestNormal(input.data(), input.size(), 4, request),
    ApduStatus::Ok);
  EXPECT_FALSE(request.hasInvocationParameter);
}

TEST(ActionCodecTest, RejectsUnsupportedActionRequestChoice)
{
  constexpr std::array<std::uint8_t, 3> input = {0xC3, 0x02, 0x81};
  ActionRequestNormal request = {};

  EXPECT_EQ(
    DecodeActionRequestNormal(input.data(), input.size(), 4, request),
    ApduStatus::UnsupportedXdlmsService);
}

TEST(ActionCodecTest, RoundTripsActionRequestNextPblock)
{
  constexpr std::array<std::uint8_t, 7> input = {
    0xC3, 0x02, 0x81, 0x00, 0x00, 0x00, 0x02};
  ActionRequest request = {};

  ASSERT_EQ(DecodeActionRequest(input.data(), input.size(), 4, request), ApduStatus::Ok);
  EXPECT_EQ(request.choice, ActionRequestChoice::NextPblock);
  EXPECT_EQ(request.blockNumber, 2U);

  std::array<std::uint8_t, 16> output = {};
  ApduWriter writer(output.data(), output.size());
  EXPECT_EQ(EncodeActionRequest(request, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), input.size());
  EXPECT_EQ(std::equal(input.begin(), input.end(), output.begin()), true);
}

TEST(ActionCodecTest, RoundTripsActionRequestWithList)
{
  constexpr std::array<std::uint8_t, 26> input = {
    0xC3, 0x03, 0x81, 0x02,
    0x00, 0x0F, 0x00, 0x00, 0x28, 0x00, 0x00, 0xFF, 0x01, 0x00,
    0x00, 0x0F, 0x00, 0x00, 0x28, 0x00, 0x00, 0xFF, 0x02, 0x01, 0x11, 0x01};
  ActionRequest request = {};

  ASSERT_EQ(DecodeActionRequest(input.data(), input.size(), 4, request), ApduStatus::Ok);
  EXPECT_EQ(request.choice, ActionRequestChoice::WithList);
  ASSERT_EQ(request.list.size(), 2U);
  EXPECT_FALSE(request.list[0].hasInvocationParameter);
  EXPECT_TRUE(request.list[1].hasInvocationParameter);
  EXPECT_EQ(request.list[1].invocationParameter.type, DlmsDataType::Unsigned);

  std::array<std::uint8_t, 32> output = {};
  ApduWriter writer(output.data(), output.size());
  EXPECT_EQ(EncodeActionRequest(request, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), input.size());
  EXPECT_EQ(std::equal(input.begin(), input.end(), output.begin()), true);
}

TEST(ActionCodecTest, RoundTripsActionRequestPblockChoices)
{
  constexpr std::array<std::uint8_t, 20> firstPblock = {
    0xC3, 0x04, 0x81,
    0x00, 0x0F, 0x00, 0x00, 0x28, 0x00, 0x00, 0xFF, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0xAA, 0xBB};
  constexpr std::array<std::uint8_t, 30> listAndFirstPblock = {
    0xC3, 0x05, 0x81, 0x02,
    0x00, 0x0F, 0x00, 0x00, 0x28, 0x00, 0x00, 0xFF, 0x01,
    0x00, 0x0F, 0x00, 0x00, 0x28, 0x00, 0x00, 0xFF, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0xAA, 0xBB};
  constexpr std::array<std::uint8_t, 11> pblock = {
    0xC3, 0x06, 0x81, 0x01, 0x00, 0x00, 0x00, 0x02, 0x02, 0xAA, 0xBB};
  ActionRequest request = {};

  ASSERT_EQ(DecodeActionRequest(firstPblock.data(), firstPblock.size(), 4, request), ApduStatus::Ok);
  EXPECT_EQ(request.choice, ActionRequestChoice::WithFirstPblock);
  EXPECT_EQ(request.dataBlock.blockNumber, 1U);

  ASSERT_EQ(
    DecodeActionRequest(listAndFirstPblock.data(), listAndFirstPblock.size(), 4, request),
    ApduStatus::Ok);
  EXPECT_EQ(request.choice, ActionRequestChoice::WithListAndFirstPblock);
  ASSERT_EQ(request.descriptorList.size(), 2U);

  ASSERT_EQ(DecodeActionRequest(pblock.data(), pblock.size(), 4, request), ApduStatus::Ok);
  EXPECT_EQ(request.choice, ActionRequestChoice::WithPblock);
  EXPECT_TRUE(request.dataBlock.lastBlock);

  std::array<std::uint8_t, 16> output = {};
  ApduWriter writer(output.data(), output.size());
  EXPECT_EQ(EncodeActionRequest(request, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), pblock.size());
  EXPECT_EQ(std::equal(pblock.begin(), pblock.end(), output.begin()), true);
}

TEST(ActionCodecTest, RoundTripsActionResponseVariants)
{
  constexpr std::array<std::uint8_t, 11> pblock = {
    0xC7, 0x02, 0x81, 0x01, 0x00, 0x00, 0x00, 0x02, 0x02, 0xAA, 0xBB};
  constexpr std::array<std::uint8_t, 11> withList = {
    0xC7, 0x03, 0x81, 0x02, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x11, 0x2A};
  constexpr std::array<std::uint8_t, 7> nextPblock = {
    0xC7, 0x04, 0x81, 0x00, 0x00, 0x00, 0x02};
  ActionResponse response = {};

  ASSERT_EQ(DecodeActionResponse(pblock.data(), pblock.size(), 4, response), ApduStatus::Ok);
  EXPECT_EQ(response.choice, ActionResponseChoice::WithPblock);
  EXPECT_TRUE(response.dataBlock.lastBlock);

  ASSERT_EQ(DecodeActionResponse(withList.data(), withList.size(), 4, response), ApduStatus::Ok);
  EXPECT_EQ(response.choice, ActionResponseChoice::WithList);
  ASSERT_EQ(response.list.size(), 2U);
  EXPECT_FALSE(response.list[0].hasReturnParameter);
  EXPECT_TRUE(response.list[1].hasReturnParameter);
  EXPECT_EQ(response.list[1].returnParameter.unsignedValue, 0x2AU);

  ASSERT_EQ(DecodeActionResponse(nextPblock.data(), nextPblock.size(), 4, response), ApduStatus::Ok);
  EXPECT_EQ(response.choice, ActionResponseChoice::NextPblock);
  EXPECT_EQ(response.blockNumber, 2U);

  std::array<std::uint8_t, 16> output = {};
  ApduWriter writer(output.data(), output.size());
  EXPECT_EQ(EncodeActionResponse(response, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), nextPblock.size());
  EXPECT_EQ(std::equal(nextPblock.begin(), nextPblock.end(), output.begin()), true);
}
