#include "dlms/apdu/get.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>

namespace {

using dlms::apdu::ApduStatus;
using dlms::apdu::ApduWriter;
using dlms::apdu::DecodeGetRequestNormal;
using dlms::apdu::DecodeGetResponseNormal;
using dlms::apdu::DlmsDataType;
using dlms::apdu::EncodeGetRequestNormal;
using dlms::apdu::EncodeGetResponseNormal;
using dlms::apdu::GetDataResultChoice;
using dlms::apdu::GetRequestNormal;
using dlms::apdu::GetResponseNormal;

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

TEST(GetCodecTest, RejectsSelectiveAccess)
{
  std::array<std::uint8_t, 13> input = kWrapperTraceGetRequest;
  input[12] = 0x01;
  GetRequestNormal request = {};

  EXPECT_EQ(
    DecodeGetRequestNormal(input.data(), input.size(), request),
    ApduStatus::UnsupportedFeature);
}
