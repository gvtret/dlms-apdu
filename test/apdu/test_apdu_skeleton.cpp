#include "dlms/apdu/apdu_types.hpp"

#include <gtest/gtest.h>

namespace {

TEST(ApduSkeleton, Builds)
{
  dlms::apdu::ByteView view = {0, 0};
  EXPECT_EQ(0u, view.size);
}

} // namespace
