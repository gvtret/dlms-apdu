#include "dlms/apdu/data.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>

namespace {

using dlms::apdu::ApduStatus;
using dlms::apdu::ApduWriter;
using dlms::apdu::DecodeDlmsData;
using dlms::apdu::DlmsData;
using dlms::apdu::DlmsDataType;
using dlms::apdu::EncodeDlmsData;

} // namespace

TEST(DlmsDataCodecTest, DecodesNullData)
{
  constexpr std::array<std::uint8_t, 1> input = {0x00};
  DlmsData data = {};

  EXPECT_EQ(DecodeDlmsData(input.data(), input.size(), 4, data), ApduStatus::Ok);
  EXPECT_EQ(data.type, DlmsDataType::NullData);
}

TEST(DlmsDataCodecTest, DecodesBoolean)
{
  constexpr std::array<std::uint8_t, 2> input = {0x03, 0x01};
  DlmsData data = {};

  EXPECT_EQ(DecodeDlmsData(input.data(), input.size(), 4, data), ApduStatus::Ok);
  EXPECT_EQ(data.type, DlmsDataType::Boolean);
  EXPECT_TRUE(data.booleanValue);
}

TEST(DlmsDataCodecTest, DecodesSignedScalars)
{
  constexpr std::array<std::uint8_t, 10> input = {
    0x02, 0x03,
    0x0F, 0xFE,
    0x10, 0xFF, 0x80,
    0x05, 0xFF, 0xFF};
  DlmsData data = {};

  EXPECT_EQ(DecodeDlmsData(input.data(), input.size(), 4, data), ApduStatus::NeedMoreData);

  constexpr std::array<std::uint8_t, 12> complete = {
    0x02, 0x03,
    0x0F, 0xFE,
    0x10, 0xFF, 0x80,
    0x05, 0xFF, 0xFF, 0xFF, 0x00};
  ASSERT_EQ(DecodeDlmsData(complete.data(), complete.size(), 4, data), ApduStatus::Ok);
  ASSERT_EQ(data.elements.size(), 3U);
  EXPECT_EQ(data.elements[0].signedValue, -2);
  EXPECT_EQ(data.elements[1].signedValue, -128);
  EXPECT_EQ(data.elements[2].signedValue, -256);
}

TEST(DlmsDataCodecTest, DecodesWrapperTraceGetResponseDoubleLongUnsigned)
{
  constexpr std::array<std::uint8_t, 5> input = {0x06, 0x00, 0x00, 0x09, 0xF1};
  DlmsData data = {};

  EXPECT_EQ(DecodeDlmsData(input.data(), input.size(), 4, data), ApduStatus::Ok);
  EXPECT_EQ(data.type, DlmsDataType::DoubleLongUnsigned);
  EXPECT_EQ(data.unsignedValue, 0x000009F1U);
}

TEST(DlmsDataCodecTest, DecodesOctetString)
{
  constexpr std::array<std::uint8_t, 5> input = {0x09, 0x03, 0x01, 0x02, 0x03};
  DlmsData data = {};

  EXPECT_EQ(DecodeDlmsData(input.data(), input.size(), 4, data), ApduStatus::Ok);
  EXPECT_EQ(data.type, DlmsDataType::OctetString);
  ASSERT_EQ(data.bytes.size, 3U);
  EXPECT_EQ(data.bytes.data[0], 0x01);
  EXPECT_EQ(data.bytes.data[2], 0x03);
}

TEST(DlmsDataCodecTest, EncodesStructureRoundtrip)
{
  constexpr std::array<std::uint8_t, 13> input = {
    0x02, 0x04,
    0x00,
    0x03, 0x00,
    0x11, 0x7F,
    0x09, 0x04, 0x10, 0x20, 0x30, 0x40};
  DlmsData data = {};
  ASSERT_EQ(DecodeDlmsData(input.data(), input.size(), 4, data), ApduStatus::Ok);

  std::array<std::uint8_t, 32> output = {};
  ApduWriter writer(output.data(), output.size());

  EXPECT_EQ(EncodeDlmsData(data, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), input.size());
  EXPECT_EQ(std::equal(input.begin(), input.end(), output.begin()), true);
}

TEST(DlmsDataCodecTest, RejectsUnsupportedTag)
{
  constexpr std::array<std::uint8_t, 1> input = {0x13};
  DlmsData data = {};

  EXPECT_EQ(DecodeDlmsData(input.data(), input.size(), 4, data), ApduStatus::UnsupportedDataType);
}

TEST(DlmsDataCodecTest, RejectsDepthLimit)
{
  constexpr std::array<std::uint8_t, 5> input = {0x01, 0x01, 0x01, 0x01, 0x00};
  DlmsData data = {};

  EXPECT_EQ(DecodeDlmsData(input.data(), input.size(), 2, data), ApduStatus::InvalidData);
}
