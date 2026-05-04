#include "dlms/apdu/axdr.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

namespace {

using dlms::apdu::ApduReader;
using dlms::apdu::ApduStatus;
using dlms::apdu::ApduWriter;
using dlms::apdu::AxdrConformance;
using dlms::apdu::AxdrOctetString;
using dlms::apdu::ReadAxdrBoolean;
using dlms::apdu::ReadAxdrConformance;
using dlms::apdu::ReadAxdrLength;
using dlms::apdu::ReadAxdrOctetString;
using dlms::apdu::ReadAxdrOptionalFlag;
using dlms::apdu::WriteAxdrBoolean;
using dlms::apdu::WriteAxdrConformance;
using dlms::apdu::WriteAxdrLength;
using dlms::apdu::WriteAxdrOctetString;
using dlms::apdu::WriteAxdrOptionalFlag;

TEST(AxdrCodecTest, ReadsOptionalFlags)
{
  const std::uint8_t bytes[] = {0x00, 0x01};
  ApduReader reader(bytes, sizeof(bytes));

  bool present = true;
  ASSERT_EQ(ApduStatus::Ok, ReadAxdrOptionalFlag(reader, present));
  EXPECT_FALSE(present);

  ASSERT_EQ(ApduStatus::Ok, ReadAxdrOptionalFlag(reader, present));
  EXPECT_TRUE(present);
}

TEST(AxdrCodecTest, RejectsInvalidOptionalFlag)
{
  const std::uint8_t bytes[] = {0x02};
  ApduReader reader(bytes, sizeof(bytes));

  bool present = false;
  EXPECT_EQ(ApduStatus::InvalidAxdr, ReadAxdrOptionalFlag(reader, present));
}

TEST(AxdrCodecTest, ReadsAndWritesBoolean)
{
  const std::uint8_t bytes[] = {0xff};
  ApduReader reader(bytes, sizeof(bytes));

  bool value = false;
  ASSERT_EQ(ApduStatus::Ok, ReadAxdrBoolean(reader, value));
  EXPECT_TRUE(value);

  std::uint8_t output[2] = {};
  ApduWriter writer(output, sizeof(output));
  EXPECT_EQ(ApduStatus::Ok, WriteAxdrBoolean(writer, false));
  EXPECT_EQ(ApduStatus::Ok, WriteAxdrOptionalFlag(writer, true));
  EXPECT_EQ(0x00u, output[0]);
  EXPECT_EQ(0x01u, output[1]);
}

TEST(AxdrCodecTest, ReadsAndWritesShortLength)
{
  const std::uint8_t bytes[] = {0x7f};
  ApduReader reader(bytes, sizeof(bytes));

  std::size_t length = 0;
  ASSERT_EQ(ApduStatus::Ok, ReadAxdrLength(reader, length));
  EXPECT_EQ(127u, length);

  std::uint8_t output[1] = {};
  ApduWriter writer(output, sizeof(output));
  ASSERT_EQ(ApduStatus::Ok, WriteAxdrLength(writer, 127));
  EXPECT_EQ(0x7fu, output[0]);
}

TEST(AxdrCodecTest, ReadsAndWritesLongLength)
{
  const std::uint8_t bytes[] = {0x82, 0x01, 0x00};
  ApduReader reader(bytes, sizeof(bytes));

  std::size_t length = 0;
  ASSERT_EQ(ApduStatus::Ok, ReadAxdrLength(reader, length));
  EXPECT_EQ(256u, length);

  std::uint8_t output[3] = {};
  ApduWriter writer(output, sizeof(output));
  ASSERT_EQ(ApduStatus::Ok, WriteAxdrLength(writer, 256));
  EXPECT_EQ(0x82u, output[0]);
  EXPECT_EQ(0x01u, output[1]);
  EXPECT_EQ(0x00u, output[2]);
}

TEST(AxdrCodecTest, RejectsNonMinimalLongLength)
{
  const std::uint8_t bytes[] = {0x81, 0x7f};
  ApduReader reader(bytes, sizeof(bytes));

  std::size_t length = 0;
  EXPECT_EQ(ApduStatus::InvalidLength, ReadAxdrLength(reader, length));
}

TEST(AxdrCodecTest, ReadsOctetString)
{
  const std::uint8_t bytes[] = {0x03, 0xaa, 0xbb, 0xcc};
  ApduReader reader(bytes, sizeof(bytes));

  AxdrOctetString value = {};
  ASSERT_EQ(ApduStatus::Ok, ReadAxdrOctetString(reader, value));
  ASSERT_EQ(3u, value.size);
  EXPECT_EQ(0xaau, value.data[0]);
  EXPECT_EQ(0xccu, value.data[2]);
}

TEST(AxdrCodecTest, WritesOctetString)
{
  const std::uint8_t payload[] = {0xaa, 0xbb};
  std::uint8_t output[3] = {};
  ApduWriter writer(output, sizeof(output));

  ASSERT_EQ(ApduStatus::Ok,
            WriteAxdrOctetString(writer, payload, sizeof(payload)));
  EXPECT_EQ(0x02u, output[0]);
  EXPECT_EQ(0xaau, output[1]);
  EXPECT_EQ(0xbbu, output[2]);
}

TEST(AxdrCodecTest, ReadsConformanceApplicationTag)
{
  const std::uint8_t bytes[] = {
    0x5f, 0x1f, 0x04, 0x00, 0x00, 0x7e, 0x1f
  };
  ApduReader reader(bytes, sizeof(bytes));

  AxdrConformance value = {};
  ASSERT_EQ(ApduStatus::Ok, ReadAxdrConformance(reader, value));
  EXPECT_EQ(0x00u, value.bytes[0]);
  EXPECT_EQ(0x7eu, value.bytes[1]);
  EXPECT_EQ(0x1fu, value.bytes[2]);
}

TEST(AxdrCodecTest, WritesConformanceApplicationTag)
{
  AxdrConformance value = {{0x00, 0x7e, 0x1f}};
  std::uint8_t output[7] = {};
  ApduWriter writer(output, sizeof(output));

  ASSERT_EQ(ApduStatus::Ok, WriteAxdrConformance(writer, value));

  const std::uint8_t expected[] = {
    0x5f, 0x1f, 0x04, 0x00, 0x00, 0x7e, 0x1f
  };
  for (std::size_t i = 0; i < sizeof(expected); ++i) {
    EXPECT_EQ(expected[i], output[i]);
  }
}

TEST(AxdrCodecTest, RejectsInvalidConformanceTag)
{
  const std::uint8_t bytes[] = {
    0x5f, 0x20, 0x04, 0x00, 0x00, 0x7e, 0x1f
  };
  ApduReader reader(bytes, sizeof(bytes));

  AxdrConformance value = {};
  EXPECT_EQ(ApduStatus::InvalidConformance,
            ReadAxdrConformance(reader, value));
}

TEST(AxdrCodecTest, SmallOutputBufferIsReportedBeforeWritingOctetString)
{
  const std::uint8_t payload[] = {0xaa, 0xbb};
  std::uint8_t output[2] = {0xee, 0xee};
  ApduWriter writer(output, sizeof(output));

  EXPECT_EQ(ApduStatus::OutputBufferTooSmall,
            WriteAxdrOctetString(writer, payload, sizeof(payload)));
  EXPECT_EQ(0u, writer.WrittenSize());
  EXPECT_EQ(0xeeu, output[0]);
  EXPECT_EQ(0xeeu, output[1]);
}

} // namespace
