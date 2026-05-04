#include "dlms/apdu/get.hpp"
#include "dlms/apdu/xdlms.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace {

using dlms::apdu::ApduStatus;
using dlms::apdu::ApduWriter;
using dlms::apdu::DecodeGetRequest;
using dlms::apdu::DecodeGetRequestNormal;
using dlms::apdu::DecodeGetResponse;
using dlms::apdu::DecodeGetResponseNormal;
using dlms::apdu::DecodeXdlmsApdu;
using dlms::apdu::DlmsDataType;
using dlms::apdu::EncodeGetRequest;
using dlms::apdu::EncodeGetRequestNormal;
using dlms::apdu::EncodeGetResponse;
using dlms::apdu::EncodeGetResponseNormal;
using dlms::apdu::EncodeXdlmsApdu;
using dlms::apdu::GetDataResultChoice;
using dlms::apdu::GetRequest;
using dlms::apdu::GetRequestChoice;
using dlms::apdu::GetRequestNormal;
using dlms::apdu::GetResponse;
using dlms::apdu::GetResponseChoice;
using dlms::apdu::GetResponseNormal;
using dlms::apdu::XdlmsApdu;
using dlms::apdu::XdlmsApduKind;

constexpr std::array<std::uint8_t, 13> kWrapperTraceGetRequest = {
  0xC0, 0x01, 0x81, 0x00, 0x07, 0x01, 0x00,
  0x63, 0x01, 0x00, 0xFF, 0x07, 0x00};

constexpr std::array<std::uint8_t, 9> kWrapperTraceGetResponse = {
  0xC4, 0x01, 0x81, 0x00, 0x06, 0x00, 0x00, 0x09, 0xF1};

} // namespace

TEST(GetCodecTest, DecodesWrapperTraceGetRequestNormal)
{
  GetRequestNormal request = {};

  EXPECT_EQ(
    DecodeGetRequestNormal(
      kWrapperTraceGetRequest.data(),
      kWrapperTraceGetRequest.size(),
      request),
    ApduStatus::Ok);
  EXPECT_EQ(request.invokeIdAndPriority, 0x81);
  EXPECT_EQ(request.descriptor.classId, 0x0007U);
  EXPECT_EQ(request.descriptor.logicalName[0], 0x01);
  EXPECT_EQ(request.descriptor.logicalName[1], 0x00);
  EXPECT_EQ(request.descriptor.logicalName[2], 0x63);
  EXPECT_EQ(request.descriptor.logicalName[3], 0x01);
  EXPECT_EQ(request.descriptor.logicalName[4], 0x00);
  EXPECT_EQ(request.descriptor.logicalName[5], 0xFF);
  EXPECT_EQ(request.descriptor.attributeId, 0x07);
  EXPECT_FALSE(request.hasSelectiveAccess);
}

TEST(GetCodecTest, EncodesWrapperTraceGetRequestNormal)
{
  GetRequestNormal request = {};
  ASSERT_EQ(
    DecodeGetRequestNormal(
      kWrapperTraceGetRequest.data(),
      kWrapperTraceGetRequest.size(),
      request),
    ApduStatus::Ok);

  std::array<std::uint8_t, 32> output = {};
  ApduWriter writer(output.data(), output.size());

  EXPECT_EQ(EncodeGetRequestNormal(request, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), kWrapperTraceGetRequest.size());
  EXPECT_EQ(std::equal(kWrapperTraceGetRequest.begin(), kWrapperTraceGetRequest.end(), output.begin()), true);
}

TEST(GetCodecTest, DecodesWrapperTraceGetResponseNormal)
{
  GetResponseNormal response = {};

  EXPECT_EQ(
    DecodeGetResponseNormal(
      kWrapperTraceGetResponse.data(),
      kWrapperTraceGetResponse.size(),
      4,
      response),
    ApduStatus::Ok);
  EXPECT_EQ(response.invokeIdAndPriority, 0x81);
  EXPECT_EQ(response.resultChoice, GetDataResultChoice::Data);
  EXPECT_EQ(response.data.type, DlmsDataType::DoubleLongUnsigned);
  EXPECT_EQ(response.data.unsignedValue, 0x000009F1U);
}

TEST(GetCodecTest, EncodesWrapperTraceGetResponseNormal)
{
  GetResponseNormal response = {};
  ASSERT_EQ(
    DecodeGetResponseNormal(
      kWrapperTraceGetResponse.data(),
      kWrapperTraceGetResponse.size(),
      4,
      response),
    ApduStatus::Ok);

  std::array<std::uint8_t, 32> output = {};
  ApduWriter writer(output.data(), output.size());

  EXPECT_EQ(EncodeGetResponseNormal(response, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), kWrapperTraceGetResponse.size());
  EXPECT_EQ(std::equal(kWrapperTraceGetResponse.begin(), kWrapperTraceGetResponse.end(), output.begin()), true);
}

TEST(GetCodecTest, DecodesDataAccessErrorResponse)
{
  constexpr std::array<std::uint8_t, 5> input = {0xC4, 0x01, 0x81, 0x01, 0x0C};
  GetResponseNormal response = {};

  EXPECT_EQ(DecodeGetResponseNormal(input.data(), input.size(), 4, response), ApduStatus::Ok);
  EXPECT_EQ(response.resultChoice, GetDataResultChoice::DataAccessError);
  EXPECT_EQ(response.dataAccessError, 0x0C);
}

TEST(GetCodecTest, RejectsUnsupportedGetRequestChoice)
{
  constexpr std::array<std::uint8_t, 3> input = {0xC0, 0x02, 0x81};
  GetRequestNormal request = {};

  EXPECT_EQ(
    DecodeGetRequestNormal(input.data(), input.size(), request),
    ApduStatus::UnsupportedXdlmsService);
}

TEST(GetCodecTest, DecodesSelectiveAccess)
{
  constexpr std::array<std::uint8_t, 15> input = {
    0xC0, 0x01, 0x81, 0x00, 0x07, 0x01, 0x00, 0x63,
    0x01, 0x00, 0xFF, 0x07, 0x01, 0x01, 0x00};
  GetRequestNormal request = {};

  EXPECT_EQ(
    DecodeGetRequestNormal(input.data(), input.size(), request),
    ApduStatus::Ok);
  EXPECT_TRUE(request.hasSelectiveAccess);
  EXPECT_EQ(request.selectiveAccess.selector, 0x01);
  EXPECT_EQ(request.selectiveAccess.parameters.type, DlmsDataType::NullData);
}

TEST(GetCodecTest, RoundTripsGetRequestNext)
{
  constexpr std::array<std::uint8_t, 7> input = {
    0xC0, 0x02, 0x81, 0x00, 0x00, 0x00, 0x02};
  GetRequest request = {};

  ASSERT_EQ(DecodeGetRequest(input.data(), input.size(), 4, request), ApduStatus::Ok);
  EXPECT_EQ(request.choice, GetRequestChoice::Next);
  EXPECT_EQ(request.invokeIdAndPriority, 0x81);
  EXPECT_EQ(request.blockNumber, 2U);

  std::array<std::uint8_t, 16> output = {};
  ApduWriter writer(output.data(), output.size());
  EXPECT_EQ(EncodeGetRequest(request, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), input.size());
  EXPECT_EQ(std::equal(input.begin(), input.end(), output.begin()), true);
}

TEST(GetCodecTest, RoundTripsGetRequestWithList)
{
  constexpr std::array<std::uint8_t, 26> input = {
    0xC0, 0x03, 0x81, 0x02,
    0x00, 0x07, 0x01, 0x00, 0x63, 0x01, 0x00, 0xFF, 0x07, 0x00,
    0x00, 0x07, 0x01, 0x00, 0x63, 0x01, 0x00, 0xFF, 0x02, 0x01, 0x01, 0x00};
  GetRequest request = {};

  ASSERT_EQ(DecodeGetRequest(input.data(), input.size(), 4, request), ApduStatus::Ok);
  EXPECT_EQ(request.choice, GetRequestChoice::WithList);
  ASSERT_EQ(request.list.size(), 2U);
  EXPECT_FALSE(request.list[0].hasSelection);
  EXPECT_TRUE(request.list[1].hasSelection);
  EXPECT_EQ(request.list[1].selection.parameters.type, DlmsDataType::NullData);

  std::array<std::uint8_t, 32> output = {};
  ApduWriter writer(output.data(), output.size());
  EXPECT_EQ(EncodeGetRequest(request, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), input.size());
  EXPECT_EQ(std::equal(input.begin(), input.end(), output.begin()), true);
}

TEST(GetCodecTest, RoundTripsGetResponseWithDataBlock)
{
  constexpr std::array<std::uint8_t, 12> input = {
    0xC4, 0x02, 0x81, 0x01, 0x00, 0x00, 0x00, 0x02, 0x03, 0xAA, 0xBB, 0xCC};
  GetResponse response = {};

  ASSERT_EQ(DecodeGetResponse(input.data(), input.size(), 4, response), ApduStatus::Ok);
  EXPECT_EQ(response.choice, GetResponseChoice::WithDataBlock);
  EXPECT_TRUE(response.dataBlock.lastBlock);
  EXPECT_EQ(response.dataBlock.blockNumber, 2U);
  EXPECT_EQ(response.dataBlock.rawData.size, 3U);

  std::array<std::uint8_t, 16> output = {};
  ApduWriter writer(output.data(), output.size());
  EXPECT_EQ(EncodeGetResponse(response, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), input.size());
  EXPECT_EQ(std::equal(input.begin(), input.end(), output.begin()), true);
}

TEST(GetCodecTest, RoundTripsGetResponseWithList)
{
  constexpr std::array<std::uint8_t, 8> input = {
    0xC4, 0x03, 0x81, 0x02, 0x00, 0x11, 0x2A, 0x01};
  constexpr std::array<std::uint8_t, 1> trailing = {0x0C};
  std::array<std::uint8_t, input.size() + trailing.size()> full = {};
  std::copy(input.begin(), input.end(), full.begin());
  std::copy(trailing.begin(), trailing.end(), full.begin() + input.size());
  GetResponse response = {};

  ASSERT_EQ(DecodeGetResponse(full.data(), full.size(), 4, response), ApduStatus::Ok);
  EXPECT_EQ(response.choice, GetResponseChoice::WithList);
  ASSERT_EQ(response.list.size(), 2U);
  EXPECT_EQ(response.list[0].choice, GetDataResultChoice::Data);
  EXPECT_EQ(response.list[0].data.unsignedValue, 0x2AU);
  EXPECT_EQ(response.list[1].choice, GetDataResultChoice::DataAccessError);
  EXPECT_EQ(response.list[1].dataAccessError, 0x0C);

  std::array<std::uint8_t, 16> output = {};
  ApduWriter writer(output.data(), output.size());
  EXPECT_EQ(EncodeGetResponse(response, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), full.size());
  EXPECT_EQ(std::equal(full.begin(), full.end(), output.begin()), true);
}

TEST(GetCodecTest, XdlmsFacadeRoundTripsGetRequestNext)
{
  constexpr std::array<std::uint8_t, 7> input = {
    0xC0, 0x02, 0x81, 0x00, 0x00, 0x00, 0x02};
  XdlmsApdu apdu = {};

  ASSERT_EQ(DecodeXdlmsApdu(input.data(), input.size(), apdu), ApduStatus::Ok);
  EXPECT_EQ(apdu.kind, XdlmsApduKind::GetRequest);
  EXPECT_EQ(apdu.getRequestAny.choice, GetRequestChoice::Next);
  EXPECT_EQ(apdu.getRequestAny.blockNumber, 2U);

  std::vector<std::uint8_t> output;
  EXPECT_EQ(EncodeXdlmsApdu(apdu, output), ApduStatus::Ok);
  ASSERT_EQ(output.size(), input.size());
  EXPECT_EQ(std::equal(input.begin(), input.end(), output.begin()), true);
}
