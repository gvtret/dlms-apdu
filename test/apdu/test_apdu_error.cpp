#include "dlms/apdu/apdu_error.hpp"

#include <gtest/gtest.h>

namespace {

using dlms::apdu::ApduStatus;
using dlms::apdu::ApduStatusName;

TEST(ApduStatusTest, ValuesMatchDocumentedOrder)
{
  EXPECT_EQ(0, static_cast<int>(ApduStatus::Ok));
  EXPECT_EQ(1, static_cast<int>(ApduStatus::NeedMoreData));
  EXPECT_EQ(2, static_cast<int>(ApduStatus::OutputBufferTooSmall));
  EXPECT_EQ(3, static_cast<int>(ApduStatus::InvalidArgument));
  EXPECT_EQ(4, static_cast<int>(ApduStatus::InvalidTag));
  EXPECT_EQ(5, static_cast<int>(ApduStatus::InvalidLength));
  EXPECT_EQ(6, static_cast<int>(ApduStatus::InvalidBer));
  EXPECT_EQ(7, static_cast<int>(ApduStatus::InvalidAxdr));
  EXPECT_EQ(8, static_cast<int>(ApduStatus::InvalidChoice));
  EXPECT_EQ(9, static_cast<int>(ApduStatus::InvalidData));
  EXPECT_EQ(10, static_cast<int>(ApduStatus::InvalidInvokeId));
  EXPECT_EQ(11, static_cast<int>(ApduStatus::InvalidDescriptor));
  EXPECT_EQ(12, static_cast<int>(ApduStatus::InvalidConformance));
  EXPECT_EQ(13, static_cast<int>(ApduStatus::UnsupportedApdu));
  EXPECT_EQ(14, static_cast<int>(ApduStatus::UnsupportedAcseField));
  EXPECT_EQ(15, static_cast<int>(ApduStatus::UnsupportedXdlmsService));
  EXPECT_EQ(16, static_cast<int>(ApduStatus::UnsupportedDataType));
  EXPECT_EQ(17, static_cast<int>(ApduStatus::UnsupportedFeature));
  EXPECT_EQ(18, static_cast<int>(ApduStatus::PduTooLarge));
  EXPECT_EQ(19, static_cast<int>(ApduStatus::InternalError));
}

TEST(ApduStatusTest, NamesAreStable)
{
  EXPECT_STREQ("Ok", ApduStatusName(ApduStatus::Ok));
  EXPECT_STREQ("NeedMoreData", ApduStatusName(ApduStatus::NeedMoreData));
  EXPECT_STREQ("OutputBufferTooSmall",
               ApduStatusName(ApduStatus::OutputBufferTooSmall));
  EXPECT_STREQ("InternalError", ApduStatusName(ApduStatus::InternalError));
}

} // namespace
