#include "dlms/apdu/apdu_reader.hpp"

#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

namespace {

using dlms::apdu::ApduReader;
using dlms::apdu::ApduStatus;

TEST(ApduReaderTest, ReadsBigEndianIntegers)
{
  const std::uint8_t bytes[] = {0x12, 0x34, 0x56, 0x78};
  ApduReader reader(bytes, sizeof(bytes));

  std::uint8_t one = 0;
  std::uint16_t two = 0;
  std::uint8_t last = 0;

  EXPECT_EQ(ApduStatus::Ok, reader.ReadU8(one));
  EXPECT_EQ(0x12u, one);
  EXPECT_EQ(ApduStatus::Ok, reader.ReadU16(two));
  EXPECT_EQ(0x3456u, two);
  EXPECT_EQ(ApduStatus::Ok, reader.ReadU8(last));
  EXPECT_EQ(0x78u, last);
  EXPECT_TRUE(reader.Empty());
}

TEST(ApduReaderTest, ReadsU32BigEndian)
{
  const std::uint8_t bytes[] = {0x01, 0x23, 0x45, 0x67};
  ApduReader reader(bytes, sizeof(bytes));

  std::uint32_t value = 0;
  EXPECT_EQ(ApduStatus::Ok, reader.ReadU32(value));
  EXPECT_EQ(0x01234567u, value);
}

TEST(ApduReaderTest, ReadBytesReturnsView)
{
  const std::uint8_t bytes[] = {0xaa, 0xbb, 0xcc};
  ApduReader reader(bytes, sizeof(bytes));

  const std::uint8_t* view = 0;
  ASSERT_EQ(ApduStatus::Ok, reader.ReadBytes(view, 2));
  ASSERT_NE(static_cast<const std::uint8_t*>(0), view);
  EXPECT_EQ(0xaau, view[0]);
  EXPECT_EQ(0xbbu, view[1]);
  EXPECT_EQ(1u, reader.Remaining());
}

TEST(ApduReaderTest, NeedMoreDataDoesNotAdvanceMultiByteRead)
{
  const std::uint8_t bytes[] = {0x12};
  ApduReader reader(bytes, sizeof(bytes));

  std::uint16_t value = 0;
  EXPECT_EQ(ApduStatus::NeedMoreData, reader.ReadU16(value));
  EXPECT_EQ(0u, reader.Position());
}

TEST(ApduReaderTest, RejectsNullInputWithNonZeroSize)
{
  ApduReader reader(0, 1);

  std::uint8_t value = 0;
  EXPECT_EQ(ApduStatus::InvalidArgument, reader.ReadU8(value));
}

} // namespace
