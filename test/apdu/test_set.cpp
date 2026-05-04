#include "dlms/apdu/set.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>

namespace {

using dlms::apdu::ApduStatus;
using dlms::apdu::ApduWriter;
using dlms::apdu::DecodeSetRequest;
using dlms::apdu::DecodeSetRequestNormal;
using dlms::apdu::DecodeSetResponse;
using dlms::apdu::DecodeSetResponseNormal;
using dlms::apdu::DlmsDataType;
using dlms::apdu::EncodeSetRequest;
using dlms::apdu::EncodeSetRequestNormal;
using dlms::apdu::EncodeSetResponse;
using dlms::apdu::EncodeSetResponseNormal;
using dlms::apdu::SetRequest;
using dlms::apdu::SetRequestChoice;
using dlms::apdu::SetRequestNormal;
using dlms::apdu::SetResponse;
using dlms::apdu::SetResponseChoice;
using dlms::apdu::SetResponseNormal;

constexpr std::array<std::uint8_t, 15> kSetRequestNormal = {
  0xC1, 0x01, 0x81, 0x00, 0x07, 0x01, 0x00, 0x63,
  0x01, 0x00, 0xFF, 0x02, 0x00, 0x11, 0x2A};

constexpr std::array<std::uint8_t, 4> kSetResponseNormal = {
  0xC5, 0x01, 0x81, 0x00};

} // namespace

TEST(SetCodecTest, DecodesSetRequestNormal)
{
  SetRequestNormal request = {};

  EXPECT_EQ(
    DecodeSetRequestNormal(kSetRequestNormal.data(), kSetRequestNormal.size(), 4, request),
    ApduStatus::Ok);
  EXPECT_EQ(request.invokeIdAndPriority, 0x81);
  EXPECT_EQ(request.descriptor.classId, 0x0007U);
  EXPECT_EQ(request.descriptor.logicalName[0], 0x01);
  EXPECT_EQ(request.descriptor.logicalName[2], 0x63);
  EXPECT_EQ(request.descriptor.logicalName[5], 0xFF);
  EXPECT_EQ(request.descriptor.attributeId, 0x02);
  EXPECT_FALSE(request.hasSelectiveAccess);
  EXPECT_EQ(request.data.type, DlmsDataType::Unsigned);
  EXPECT_EQ(request.data.unsignedValue, 0x2AU);
}

TEST(SetCodecTest, EncodesSetRequestNormal)
{
  SetRequestNormal request = {};
  ASSERT_EQ(
    DecodeSetRequestNormal(kSetRequestNormal.data(), kSetRequestNormal.size(), 4, request),
    ApduStatus::Ok);

  std::array<std::uint8_t, 32> output = {};
  ApduWriter writer(output.data(), output.size());

  EXPECT_EQ(EncodeSetRequestNormal(request, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), kSetRequestNormal.size());
  EXPECT_EQ(std::equal(kSetRequestNormal.begin(), kSetRequestNormal.end(), output.begin()), true);
}

TEST(SetCodecTest, DecodesSetResponseNormal)
{
  SetResponseNormal response = {};

  EXPECT_EQ(
    DecodeSetResponseNormal(kSetResponseNormal.data(), kSetResponseNormal.size(), response),
    ApduStatus::Ok);
  EXPECT_EQ(response.invokeIdAndPriority, 0x81);
  EXPECT_EQ(response.result, 0x00);
}

TEST(SetCodecTest, EncodesSetResponseNormal)
{
  SetResponseNormal response = {};
  ASSERT_EQ(
    DecodeSetResponseNormal(kSetResponseNormal.data(), kSetResponseNormal.size(), response),
    ApduStatus::Ok);

  std::array<std::uint8_t, 8> output = {};
  ApduWriter writer(output.data(), output.size());

  EXPECT_EQ(EncodeSetResponseNormal(response, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), kSetResponseNormal.size());
  EXPECT_EQ(std::equal(kSetResponseNormal.begin(), kSetResponseNormal.end(), output.begin()), true);
}

TEST(SetCodecTest, RejectsUnsupportedSetRequestChoice)
{
  constexpr std::array<std::uint8_t, 3> input = {0xC1, 0x02, 0x81};
  SetRequestNormal request = {};

  EXPECT_EQ(
    DecodeSetRequestNormal(input.data(), input.size(), 4, request),
    ApduStatus::UnsupportedXdlmsService);
}

TEST(SetCodecTest, DecodesSelectiveAccess)
{
  constexpr std::array<std::uint8_t, 17> input = {
    0xC1, 0x01, 0x81, 0x00, 0x07, 0x01, 0x00, 0x63,
    0x01, 0x00, 0xFF, 0x02, 0x01, 0x01, 0x00, 0x11, 0x2A};
  SetRequestNormal request = {};

  EXPECT_EQ(
    DecodeSetRequestNormal(input.data(), input.size(), 4, request),
    ApduStatus::Ok);
  EXPECT_TRUE(request.hasSelectiveAccess);
  EXPECT_EQ(request.selectiveAccess.selector, 0x01);
  EXPECT_EQ(request.selectiveAccess.parameters.type, DlmsDataType::NullData);
  EXPECT_EQ(request.data.type, DlmsDataType::Unsigned);
  EXPECT_EQ(request.data.unsignedValue, 0x2AU);
}

TEST(SetCodecTest, RoundTripsSetRequestWithFirstDataBlock)
{
  constexpr std::array<std::uint8_t, 21> input = {
    0xC1, 0x02, 0x81,
    0x00, 0x07, 0x01, 0x00, 0x63, 0x01, 0x00, 0xFF, 0x02, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0xAA, 0xBB};
  SetRequest request = {};

  ASSERT_EQ(DecodeSetRequest(input.data(), input.size(), 4, request), ApduStatus::Ok);
  EXPECT_EQ(request.choice, SetRequestChoice::WithFirstDataBlock);
  EXPECT_EQ(request.dataBlock.blockNumber, 1U);
  EXPECT_EQ(request.dataBlock.rawData.size, 2U);

  std::array<std::uint8_t, 32> output = {};
  ApduWriter writer(output.data(), output.size());
  EXPECT_EQ(EncodeSetRequest(request, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), input.size());
  EXPECT_EQ(std::equal(input.begin(), input.end(), output.begin()), true);
}

TEST(SetCodecTest, RoundTripsSetRequestWithDataBlock)
{
  constexpr std::array<std::uint8_t, 10> input = {
    0xC1, 0x03, 0x81, 0x01, 0x00, 0x00, 0x00, 0x02, 0x01, 0xCC};
  SetRequest request = {};

  ASSERT_EQ(DecodeSetRequest(input.data(), input.size(), 4, request), ApduStatus::Ok);
  EXPECT_EQ(request.choice, SetRequestChoice::WithDataBlock);
  EXPECT_TRUE(request.dataBlock.lastBlock);
  EXPECT_EQ(request.dataBlock.blockNumber, 2U);

  std::array<std::uint8_t, 16> output = {};
  ApduWriter writer(output.data(), output.size());
  EXPECT_EQ(EncodeSetRequest(request, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), input.size());
  EXPECT_EQ(std::equal(input.begin(), input.end(), output.begin()), true);
}

TEST(SetCodecTest, RoundTripsSetRequestWithList)
{
  constexpr std::array<std::uint8_t, 28> input = {
    0xC1, 0x04, 0x81, 0x02,
    0x00, 0x07, 0x01, 0x00, 0x63, 0x01, 0x00, 0xFF, 0x02, 0x00,
    0x00, 0x07, 0x01, 0x00, 0x63, 0x01, 0x00, 0xFF, 0x03, 0x00,
    0x02, 0x11, 0x01, 0x00};
  SetRequest request = {};

  ASSERT_EQ(DecodeSetRequest(input.data(), input.size(), 4, request), ApduStatus::Ok);
  EXPECT_EQ(request.choice, SetRequestChoice::WithList);
  ASSERT_EQ(request.list.size(), 2U);
  ASSERT_EQ(request.valueList.size(), 2U);
  EXPECT_EQ(request.valueList[0].type, DlmsDataType::Unsigned);
  EXPECT_EQ(request.valueList[1].type, DlmsDataType::NullData);

  std::array<std::uint8_t, 32> output = {};
  ApduWriter writer(output.data(), output.size());
  EXPECT_EQ(EncodeSetRequest(request, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), input.size());
  EXPECT_EQ(std::equal(input.begin(), input.end(), output.begin()), true);
}

TEST(SetCodecTest, RoundTripsSetRequestWithListAndFirstDataBlock)
{
  constexpr std::array<std::uint8_t, 32> input = {
    0xC1, 0x05, 0x81, 0x02,
    0x00, 0x07, 0x01, 0x00, 0x63, 0x01, 0x00, 0xFF, 0x02, 0x00,
    0x00, 0x07, 0x01, 0x00, 0x63, 0x01, 0x00, 0xFF, 0x03, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0xAA, 0xBB};
  SetRequest request = {};

  ASSERT_EQ(DecodeSetRequest(input.data(), input.size(), 4, request), ApduStatus::Ok);
  EXPECT_EQ(request.choice, SetRequestChoice::WithListAndFirstDataBlock);
  ASSERT_EQ(request.list.size(), 2U);
  EXPECT_EQ(request.dataBlock.blockNumber, 1U);

  std::array<std::uint8_t, 40> output = {};
  ApduWriter writer(output.data(), output.size());
  EXPECT_EQ(EncodeSetRequest(request, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), input.size());
  EXPECT_EQ(std::equal(input.begin(), input.end(), output.begin()), true);
}

TEST(SetCodecTest, RoundTripsSetResponseBlockChoices)
{
  constexpr std::array<std::uint8_t, 7> dataBlock = {
    0xC5, 0x02, 0x81, 0x00, 0x00, 0x00, 0x02};
  constexpr std::array<std::uint8_t, 8> lastDataBlock = {
    0xC5, 0x03, 0x81, 0x00, 0x00, 0x00, 0x00, 0x02};
  constexpr std::array<std::uint8_t, 10> lastDataBlockWithList = {
    0xC5, 0x04, 0x81, 0x02, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x02};
  constexpr std::array<std::uint8_t, 6> withList = {
    0xC5, 0x05, 0x81, 0x02, 0x00, 0x0C};
  SetResponse response = {};

  ASSERT_EQ(DecodeSetResponse(dataBlock.data(), dataBlock.size(), response), ApduStatus::Ok);
  EXPECT_EQ(response.choice, SetResponseChoice::DataBlock);
  EXPECT_EQ(response.blockNumber, 2U);

  ASSERT_EQ(DecodeSetResponse(lastDataBlock.data(), lastDataBlock.size(), response), ApduStatus::Ok);
  EXPECT_EQ(response.choice, SetResponseChoice::LastDataBlock);
  EXPECT_EQ(response.result, 0x00);
  EXPECT_EQ(response.blockNumber, 2U);

  ASSERT_EQ(
    DecodeSetResponse(lastDataBlockWithList.data(), lastDataBlockWithList.size(), response),
    ApduStatus::Ok);
  EXPECT_EQ(response.choice, SetResponseChoice::LastDataBlockWithList);
  ASSERT_EQ(response.resultList.size(), 2U);
  EXPECT_EQ(response.blockNumber, 2U);

  ASSERT_EQ(DecodeSetResponse(withList.data(), withList.size(), response), ApduStatus::Ok);
  EXPECT_EQ(response.choice, SetResponseChoice::WithList);
  ASSERT_EQ(response.resultList.size(), 2U);

  std::array<std::uint8_t, 16> output = {};
  ApduWriter writer(output.data(), output.size());
  EXPECT_EQ(EncodeSetResponse(response, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), withList.size());
  EXPECT_EQ(std::equal(withList.begin(), withList.end(), output.begin()), true);
}
