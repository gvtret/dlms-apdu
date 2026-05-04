#include "dlms/apdu/ber.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

namespace {

using dlms::apdu::ApduReader;
using dlms::apdu::ApduStatus;
using dlms::apdu::ApduWriter;
using dlms::apdu::BerTlv;
using dlms::apdu::ReadBerLength;
using dlms::apdu::ReadBerTlv;
using dlms::apdu::ValidateBerTlv;
using dlms::apdu::WriteBerLength;
using dlms::apdu::WriteBerTlv;

TEST(BerCodecTest, DecodeShortLengthTlv)
{
  const std::uint8_t bytes[] = {0x04, 0x03, 0xaa, 0xbb, 0xcc};
  ApduReader reader(bytes, sizeof(bytes));

  BerTlv tlv = {};
  ASSERT_EQ(ApduStatus::Ok, ReadBerTlv(reader, tlv));

  EXPECT_EQ(0x04u, tlv.tag);
  ASSERT_EQ(3u, tlv.valueSize);
  EXPECT_EQ(0xaau, tlv.value[0]);
  EXPECT_EQ(0xbbu, tlv.value[1]);
  EXPECT_EQ(0xccu, tlv.value[2]);
  EXPECT_EQ(2u, tlv.headerSize);
  EXPECT_TRUE(reader.Empty());
}

TEST(BerCodecTest, DecodeLongLengthTlv)
{
  std::vector<std::uint8_t> bytes;
  bytes.push_back(0x04);
  bytes.push_back(0x81);
  bytes.push_back(0x80);
  bytes.resize(3 + 128, 0x5a);

  ApduReader reader(&bytes[0], bytes.size());
  BerTlv tlv = {};
  ASSERT_EQ(ApduStatus::Ok, ReadBerTlv(reader, tlv));

  EXPECT_EQ(0x04u, tlv.tag);
  EXPECT_EQ(128u, tlv.valueSize);
  EXPECT_EQ(3u, tlv.headerSize);
  EXPECT_EQ(0x5au, tlv.value[127]);
}

TEST(BerCodecTest, RejectsIndefiniteLength)
{
  const std::uint8_t bytes[] = {0x80};
  ApduReader reader(bytes, sizeof(bytes));

  std::size_t length = 0;
  EXPECT_EQ(ApduStatus::UnsupportedFeature, ReadBerLength(reader, length));
}

TEST(BerCodecTest, RejectsNonMinimalLongLength)
{
  const std::uint8_t bytes[] = {0x81, 0x7f};
  ApduReader reader(bytes, sizeof(bytes));

  std::size_t length = 0;
  EXPECT_EQ(ApduStatus::InvalidLength, ReadBerLength(reader, length));
}

TEST(BerCodecTest, RejectsHighTagNumberForm)
{
  const std::uint8_t bytes[] = {0x1f, 0x01, 0x00};
  ApduReader reader(bytes, sizeof(bytes));

  BerTlv tlv = {};
  EXPECT_EQ(ApduStatus::UnsupportedFeature, ReadBerTlv(reader, tlv));
}

TEST(BerCodecTest, ReportsNeedMoreDataForTruncatedValue)
{
  const std::uint8_t bytes[] = {0x04, 0x03, 0xaa};
  ApduReader reader(bytes, sizeof(bytes));

  BerTlv tlv = {};
  EXPECT_EQ(ApduStatus::NeedMoreData, ReadBerTlv(reader, tlv));
}

TEST(BerCodecTest, EncodesShortLengthTlv)
{
  const std::uint8_t value[] = {0xaa, 0xbb};
  std::uint8_t output[4] = {};
  ApduWriter writer(output, sizeof(output));

  ASSERT_EQ(ApduStatus::Ok, WriteBerTlv(writer, 0x04, value, sizeof(value)));

  const std::uint8_t expected[] = {0x04, 0x02, 0xaa, 0xbb};
  ASSERT_EQ(sizeof(expected), writer.WrittenSize());
  for (std::size_t i = 0; i < sizeof(expected); ++i) {
    EXPECT_EQ(expected[i], output[i]);
  }
}

TEST(BerCodecTest, EncodesLongLength)
{
  std::uint8_t output[3] = {};
  ApduWriter writer(output, sizeof(output));

  ASSERT_EQ(ApduStatus::Ok, WriteBerLength(writer, 0x0102));

  const std::uint8_t expected[] = {0x82, 0x01, 0x02};
  for (std::size_t i = 0; i < sizeof(expected); ++i) {
    EXPECT_EQ(expected[i], output[i]);
  }
}

TEST(BerCodecTest, NestedSequenceValidates)
{
  const std::uint8_t bytes[] = {
    0x30, 0x05,
      0x30, 0x03,
        0x05, 0x01, 0x00
  };

  EXPECT_EQ(ApduStatus::Ok, ValidateBerTlv(bytes, sizeof(bytes), 2));
}

TEST(BerCodecTest, DepthLimitRejectsNestedSequence)
{
  const std::uint8_t bytes[] = {
    0x30, 0x05,
      0x30, 0x03,
        0x05, 0x01, 0x00
  };

  EXPECT_EQ(ApduStatus::InvalidBer, ValidateBerTlv(bytes, sizeof(bytes), 1));
}

TEST(BerCodecTest, ObjectIdentifierTlvDecodes)
{
  const std::uint8_t bytes[] = {
    0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01
  };
  ApduReader reader(bytes, sizeof(bytes));

  BerTlv tlv = {};
  ASSERT_EQ(ApduStatus::Ok, ReadBerTlv(reader, tlv));

  EXPECT_EQ(0x06u, tlv.tag);
  EXPECT_EQ(7u, tlv.valueSize);
  EXPECT_EQ(0x60u, tlv.value[0]);
  EXPECT_EQ(0x01u, tlv.value[6]);
}

TEST(BerCodecTest, SmallOutputBufferIsReported)
{
  const std::uint8_t value[] = {0xaa, 0xbb};
  std::uint8_t output[3] = {0xee, 0xee, 0xee};
  ApduWriter writer(output, sizeof(output));

  EXPECT_EQ(ApduStatus::OutputBufferTooSmall,
            WriteBerTlv(writer, 0x04, value, sizeof(value)));
  EXPECT_EQ(0u, writer.WrittenSize());
  EXPECT_EQ(0xeeu, output[0]);
  EXPECT_EQ(0xeeu, output[1]);
  EXPECT_EQ(0xeeu, output[2]);
}

} // namespace
