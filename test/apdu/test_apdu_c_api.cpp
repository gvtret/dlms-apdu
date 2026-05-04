#include "dlms/apdu/apdu_c_api.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

extern "C" int dlms_apdu_c_header_compiles_as_c(void);

namespace {

TEST(ApduCApiTest, HeaderCompilesAsC)
{
  EXPECT_EQ(2, dlms_apdu_c_header_compiles_as_c());
}

TEST(ApduCApiTest, DecodesXdlmsView)
{
  constexpr std::array<std::uint8_t, 13> input = {
    0xC0, 0x01, 0x81, 0x00, 0x07, 0x01, 0x00,
    0x63, 0x01, 0x00, 0xFF, 0x07, 0x00};
  dlms_apdu_xdlms_t apdu = {};

  ASSERT_EQ(DLMS_APDU_STATUS_OK, dlms_apdu_decode_xdlms(input.data(), input.size(), &apdu));
  EXPECT_EQ(DLMS_APDU_XDLMS_GET_REQUEST, apdu.kind);
  EXPECT_EQ(0xC0, apdu.tag);
  ASSERT_EQ(input.size() - 1U, apdu.payload_size);
  EXPECT_EQ(std::equal(input.begin() + 1, input.end(), apdu.payload), true);
}

TEST(ApduCApiTest, EncodesXdlmsView)
{
  constexpr std::array<std::uint8_t, 6> payload = {
    0x01, 0x81, 0x00, 0x00, 0x00, 0x02};
  dlms_apdu_xdlms_t apdu = {};
  apdu.kind = DLMS_APDU_XDLMS_GET_REQUEST;
  apdu.tag = 0xC0;
  apdu.payload = payload.data();
  apdu.payload_size = payload.size();

  std::array<std::uint8_t, 8> output = {};
  std::size_t written_size = 0;
  ASSERT_EQ(
    DLMS_APDU_STATUS_OK,
    dlms_apdu_encode_xdlms(&apdu, output.data(), output.size(), &written_size));
  ASSERT_EQ(payload.size() + 1U, written_size);
  EXPECT_EQ(0xC0, output[0]);
  EXPECT_EQ(std::equal(payload.begin(), payload.end(), output.begin() + 1), true);
}

TEST(ApduCApiTest, ReportsSmallOutputBuffer)
{
  constexpr std::array<std::uint8_t, 2> payload = {0x01, 0x81};
  dlms_apdu_xdlms_t apdu = {};
  apdu.kind = DLMS_APDU_XDLMS_GET_REQUEST;
  apdu.tag = 0xC0;
  apdu.payload = payload.data();
  apdu.payload_size = payload.size();

  std::array<std::uint8_t, 2> output = {};
  std::size_t written_size = 7;
  EXPECT_EQ(
    DLMS_APDU_STATUS_OUTPUT_BUFFER_TOO_SMALL,
    dlms_apdu_encode_xdlms(&apdu, output.data(), output.size(), &written_size));
  EXPECT_EQ(0U, written_size);
}

TEST(ApduCApiTest, ValidatesNullArguments)
{
  constexpr std::array<std::uint8_t, 1> input = {0xC0};
  dlms_apdu_xdlms_t apdu = {};
  std::array<std::uint8_t, 4> output = {};
  std::size_t written_size = 0;

  EXPECT_EQ(DLMS_APDU_STATUS_INVALID_ARGUMENT, dlms_apdu_decode_xdlms(input.data(), input.size(), 0));
  EXPECT_EQ(DLMS_APDU_STATUS_INVALID_ARGUMENT, dlms_apdu_decode_xdlms(0, input.size(), &apdu));
  EXPECT_EQ(DLMS_APDU_STATUS_NEED_MORE_DATA, dlms_apdu_decode_xdlms(0, 0, &apdu));
  EXPECT_EQ(DLMS_APDU_STATUS_INVALID_ARGUMENT, dlms_apdu_encode_xdlms(0, output.data(), output.size(), &written_size));
  EXPECT_EQ(DLMS_APDU_STATUS_INVALID_ARGUMENT, dlms_apdu_encode_xdlms(&apdu, output.data(), output.size(), 0));

  apdu.tag = 0xC0;
  apdu.payload = 0;
  apdu.payload_size = 1;
  EXPECT_EQ(DLMS_APDU_STATUS_INVALID_ARGUMENT, dlms_apdu_encode_xdlms(&apdu, output.data(), output.size(), &written_size));
}

} // namespace
