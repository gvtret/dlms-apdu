#include "dlms/apdu/apdu_writer.hpp"

#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

namespace {

using dlms::apdu::ApduStatus;
using dlms::apdu::ApduWriter;

TEST(ApduWriterTest, WritesBigEndianIntegers)
{
  std::uint8_t bytes[7] = {};
  ApduWriter writer(bytes, sizeof(bytes));

  EXPECT_EQ(ApduStatus::Ok, writer.WriteU8(0x12));
  EXPECT_EQ(ApduStatus::Ok, writer.WriteU16(0x3456));
  EXPECT_EQ(ApduStatus::Ok, writer.WriteU32(0x789abcde));

  const std::uint8_t expected[] = {
    0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde
  };
  for (std::size_t i = 0; i < sizeof(expected); ++i) {
    EXPECT_EQ(expected[i], bytes[i]);
  }
  EXPECT_EQ(sizeof(expected), writer.WrittenSize());
}

TEST(ApduWriterTest, WritesBytes)
{
  const std::uint8_t payload[] = {0xaa, 0xbb, 0xcc};
  std::uint8_t bytes[3] = {};
  ApduWriter writer(bytes, sizeof(bytes));

  EXPECT_EQ(ApduStatus::Ok, writer.WriteBytes(payload, sizeof(payload)));
  EXPECT_EQ(0xaau, bytes[0]);
  EXPECT_EQ(0xbbu, bytes[1]);
  EXPECT_EQ(0xccu, bytes[2]);
}

TEST(ApduWriterTest, SmallBufferDoesNotAdvanceMultiByteWrite)
{
  std::uint8_t bytes[1] = {0xee};
  ApduWriter writer(bytes, sizeof(bytes));

  EXPECT_EQ(ApduStatus::OutputBufferTooSmall, writer.WriteU16(0x1234));
  EXPECT_EQ(0u, writer.WrittenSize());
  EXPECT_EQ(0xeeu, bytes[0]);
}

TEST(ApduWriterTest, RejectsNullOutputWithNonZeroSize)
{
  ApduWriter writer(0, 1);

  EXPECT_EQ(ApduStatus::InvalidArgument, writer.WriteU8(0x12));
}

TEST(ApduWriterTest, RejectsNullBytesWithNonZeroSize)
{
  std::uint8_t bytes[1] = {};
  ApduWriter writer(bytes, sizeof(bytes));

  EXPECT_EQ(ApduStatus::InvalidArgument, writer.WriteBytes(0, 1));
}

} // namespace
