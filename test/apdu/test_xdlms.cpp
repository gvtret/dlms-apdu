#include "dlms/apdu/xdlms.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace {

using dlms::apdu::ApduStatus;
using dlms::apdu::CipheredApduKind;
using dlms::apdu::DecodeXdlmsApdu;
using dlms::apdu::EncodeXdlmsApdu;
using dlms::apdu::XdlmsApdu;
using dlms::apdu::XdlmsApduKind;

} // namespace

TEST(XdlmsCodecTest, DecodesAndEncodesServiceSpecificCipheredApduAsOpaque)
{
  constexpr std::array<std::uint8_t, 7> input = {
    0xC8, 0x05, 0x10, 0x01, 0x23, 0x45, 0x67};
  XdlmsApdu apdu = {};

  ASSERT_EQ(DecodeXdlmsApdu(input.data(), input.size(), apdu), ApduStatus::Ok);
  EXPECT_EQ(apdu.kind, XdlmsApduKind::Ciphered);
  EXPECT_EQ(apdu.ciphered.kind, CipheredApduKind::ServiceSpecific);
  EXPECT_EQ(apdu.ciphered.tag, 0xC8);
  ASSERT_EQ(apdu.ciphered.payload.size, input.size() - 1U);
  EXPECT_EQ(std::equal(input.begin() + 1, input.end(), apdu.ciphered.payload.data), true);

  std::vector<std::uint8_t> output;
  EXPECT_EQ(EncodeXdlmsApdu(apdu, output), ApduStatus::Ok);
  ASSERT_EQ(output.size(), input.size());
  EXPECT_EQ(std::equal(input.begin(), input.end(), output.begin()), true);
}

TEST(XdlmsCodecTest, DecodesGeneralCipheringTagsAsOpaque)
{
  constexpr std::array<std::uint8_t, 3> generalGlo = {0xDB, 0x01, 0xAA};
  constexpr std::array<std::uint8_t, 3> generalDed = {0xDC, 0x01, 0xBB};
  constexpr std::array<std::uint8_t, 3> general = {0xDD, 0x01, 0xCC};
  XdlmsApdu apdu = {};

  ASSERT_EQ(DecodeXdlmsApdu(generalGlo.data(), generalGlo.size(), apdu), ApduStatus::Ok);
  EXPECT_EQ(apdu.kind, XdlmsApduKind::Ciphered);
  EXPECT_EQ(apdu.ciphered.kind, CipheredApduKind::GeneralGloCiphering);

  ASSERT_EQ(DecodeXdlmsApdu(generalDed.data(), generalDed.size(), apdu), ApduStatus::Ok);
  EXPECT_EQ(apdu.kind, XdlmsApduKind::Ciphered);
  EXPECT_EQ(apdu.ciphered.kind, CipheredApduKind::GeneralDedCiphering);

  ASSERT_EQ(DecodeXdlmsApdu(general.data(), general.size(), apdu), ApduStatus::Ok);
  EXPECT_EQ(apdu.kind, XdlmsApduKind::Ciphered);
  EXPECT_EQ(apdu.ciphered.kind, CipheredApduKind::GeneralCiphering);
}

TEST(XdlmsCodecTest, RejectsUnsupportedTopLevelTag)
{
  constexpr std::array<std::uint8_t, 2> input = {0xFE, 0x00};
  XdlmsApdu apdu = {};

  EXPECT_EQ(DecodeXdlmsApdu(input.data(), input.size(), apdu), ApduStatus::UnsupportedXdlmsService);
}
