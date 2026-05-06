#include "dlms/apdu/acse.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace {

using dlms::apdu::AareApdu;
using dlms::apdu::AarqApdu;
using dlms::apdu::ApduStatus;
using dlms::apdu::ApduWriter;
using dlms::apdu::DecodeAare;
using dlms::apdu::DecodeAarq;
using dlms::apdu::DecodeAcseApdu;
using dlms::apdu::DecodeRlre;
using dlms::apdu::DecodeRlrq;
using dlms::apdu::EncodeAare;
using dlms::apdu::EncodeAarq;
using dlms::apdu::EncodeAcseApdu;
using dlms::apdu::EncodeRlre;
using dlms::apdu::EncodeRlrq;

constexpr std::array<std::uint8_t, 68> kSpodesAarq = {
  0x60, 0x42, 0x80, 0x02, 0x02, 0x84, 0xA1, 0x09, 0x06, 0x07, 0x60, 0x85,
  0x74, 0x05, 0x08, 0x01, 0x01, 0x8A, 0x02, 0x07, 0x80, 0x8B, 0x07, 0x60,
  0x85, 0x74, 0x05, 0x08, 0x02, 0x02, 0xAC, 0x12, 0x80, 0x10, 0x16, 0x1E,
  0x69, 0x35, 0x35, 0x25, 0x6C, 0x25, 0x42, 0x52, 0x6B, 0x48, 0x26, 0x72,
  0x17, 0x42, 0xBE, 0x10, 0x04, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x06, 0x5F,
  0x1F, 0x04, 0x00, 0x62, 0x1E, 0x5D, 0x02, 0x00};

constexpr std::array<std::uint8_t, 80> kSpodesAare = {
  0x61, 0x4E, 0x80, 0x02, 0x02, 0x84, 0xA1, 0x09, 0x06, 0x07, 0x60, 0x85,
  0x74, 0x05, 0x08, 0x01, 0x01, 0xA2, 0x03, 0x02, 0x01, 0x00, 0xA3, 0x05,
  0xA1, 0x03, 0x02, 0x01, 0x0E, 0x88, 0x02, 0x07, 0x80, 0x89, 0x07, 0x60,
  0x85, 0x74, 0x05, 0x08, 0x02, 0x02, 0xAA, 0x12, 0x80, 0x10, 0xC6, 0x69,
  0x73, 0x51, 0xFF, 0x4A, 0xEC, 0x29, 0xCD, 0xBA, 0xAB, 0xF2, 0xFB, 0xE3,
  0x46, 0x7C, 0xBE, 0x10, 0x04, 0x0E, 0x08, 0x00, 0x06, 0x5F, 0x1F, 0x04,
  0x00, 0x40, 0x18, 0x1D, 0x02, 0x00, 0x00, 0x07};

} // namespace

TEST(AcseTest, DecodeSpodesAarqFromHdlcTrace)
{
  AarqApdu aarq = {};

  EXPECT_EQ(DecodeAarq(kSpodesAarq.data(), kSpodesAarq.size(), aarq), ApduStatus::Ok);
  ASSERT_EQ(aarq.fields.size(), 5U);
  EXPECT_EQ(aarq.fields[0].tag, 0x80);
  EXPECT_EQ(aarq.fields[4].tag, 0xAC);
  EXPECT_EQ(aarq.initiateRequest.proposedDlmsVersionNumber, 6U);
  EXPECT_EQ(aarq.initiateRequest.proposedConformance.bytes[0], 0x62);
  EXPECT_EQ(aarq.initiateRequest.proposedConformance.bytes[1], 0x1E);
  EXPECT_EQ(aarq.initiateRequest.proposedConformance.bytes[2], 0x5D);
  EXPECT_EQ(aarq.initiateRequest.clientMaxReceivePduSize, 0x0200U);
}

TEST(AcseTest, EncodeSpodesAarqRoundTrips)
{
  AarqApdu aarq = {};
  ASSERT_EQ(DecodeAarq(kSpodesAarq.data(), kSpodesAarq.size(), aarq), ApduStatus::Ok);

  std::array<std::uint8_t, 128> output = {};
  ApduWriter writer(output.data(), output.size());

  EXPECT_EQ(EncodeAarq(aarq, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), kSpodesAarq.size());
  EXPECT_EQ(std::equal(kSpodesAarq.begin(), kSpodesAarq.end(), output.begin()), true);
}

TEST(AcseTest, DecodeSpodesAareFromHdlcTrace)
{
  AareApdu aare = {};

  EXPECT_EQ(DecodeAare(kSpodesAare.data(), kSpodesAare.size(), aare), ApduStatus::Ok);
  ASSERT_EQ(aare.fields.size(), 7U);
  EXPECT_TRUE(aare.hasResult);
  EXPECT_EQ(aare.result, 0);
  EXPECT_TRUE(aare.hasDiagnostic);
  EXPECT_EQ(aare.diagnostic, 14);
  EXPECT_EQ(aare.initiateResponse.negotiatedDlmsVersionNumber, 6U);
  EXPECT_EQ(aare.initiateResponse.negotiatedConformance.bytes[0], 0x40);
  EXPECT_EQ(aare.initiateResponse.negotiatedConformance.bytes[1], 0x18);
  EXPECT_EQ(aare.initiateResponse.negotiatedConformance.bytes[2], 0x1D);
  EXPECT_EQ(aare.initiateResponse.serverMaxReceivePduSize, 0x0200U);
  EXPECT_EQ(aare.initiateResponse.vaaName, 0x0007U);
}

TEST(AcseTest, EncodeSpodesAareRoundTrips)
{
  AareApdu aare = {};
  ASSERT_EQ(DecodeAare(kSpodesAare.data(), kSpodesAare.size(), aare), ApduStatus::Ok);

  std::array<std::uint8_t, 128> output = {};
  ApduWriter writer(output.data(), output.size());

  EXPECT_EQ(EncodeAare(aare, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), kSpodesAare.size());
  EXPECT_EQ(std::equal(kSpodesAare.begin(), kSpodesAare.end(), output.begin()), true);
}

TEST(AcseTest, DecodeAarqRejectsWrongTopLevelTag)
{
  std::array<std::uint8_t, 4> input = {0x61, 0x02, 0x01, 0x00};
  AarqApdu aarq = {};

  EXPECT_EQ(DecodeAarq(input.data(), input.size(), aarq), ApduStatus::InvalidTag);
}

TEST(AcseTest, DecodeAareRejectsMissingUserInformation)
{
  std::array<std::uint8_t, 7> input = {0x61, 0x05, 0xA2, 0x03, 0x02, 0x01, 0x00};
  AareApdu aare = {};

  EXPECT_EQ(DecodeAare(input.data(), input.size(), aare), ApduStatus::InvalidData);
}

TEST(AcseTest, EncodeAarqRejectsRawUserInformationField)
{
  AarqApdu aarq = {};
  aarq.fields.push_back(dlms::apdu::AcseRawField{0xBE, {kSpodesAarq.data(), 2}});

  std::array<std::uint8_t, 128> output = {};
  ApduWriter writer(output.data(), output.size());

  EXPECT_EQ(EncodeAarq(aarq, writer), ApduStatus::InvalidArgument);
}

TEST(AcseTest, DecodeEmptyRlrq)
{
  const std::array<std::uint8_t, 2> input = {0x62, 0x00};
  dlms::apdu::RlrqApdu rlrq = {};

  EXPECT_EQ(DecodeRlrq(input.data(), input.size(), rlrq), ApduStatus::Ok);
  EXPECT_FALSE(rlrq.hasReason);
  EXPECT_TRUE(rlrq.fields.empty());
}

TEST(AcseTest, EncodeEmptyRlrq)
{
  dlms::apdu::RlrqApdu rlrq = {};
  std::array<std::uint8_t, 8> output = {};
  ApduWriter writer(output.data(), output.size());

  EXPECT_EQ(EncodeRlrq(rlrq, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), 2U);
  EXPECT_EQ(output[0], 0x62);
  EXPECT_EQ(output[1], 0x00);
}

TEST(AcseTest, DecodeRlreWithReason)
{
  const std::array<std::uint8_t, 5> input = {0x63, 0x03, 0x80, 0x01, 0x00};
  dlms::apdu::RlreApdu rlre = {};

  EXPECT_EQ(DecodeRlre(input.data(), input.size(), rlre), ApduStatus::Ok);
  EXPECT_TRUE(rlre.hasReason);
  EXPECT_EQ(rlre.reason, 0);
  ASSERT_EQ(rlre.fields.size(), 1U);
  EXPECT_EQ(rlre.fields[0].tag, 0x80);
}

TEST(AcseTest, EncodeRlreWithReason)
{
  dlms::apdu::RlreApdu rlre = {};
  rlre.hasReason = true;
  rlre.reason = 0;

  std::array<std::uint8_t, 8> output = {};
  ApduWriter writer(output.data(), output.size());

  EXPECT_EQ(EncodeRlre(rlre, writer), ApduStatus::Ok);
  ASSERT_EQ(writer.WrittenSize(), 5U);
  EXPECT_EQ(output[0], 0x63);
  EXPECT_EQ(output[1], 0x03);
  EXPECT_EQ(output[2], 0x80);
  EXPECT_EQ(output[3], 0x01);
  EXPECT_EQ(output[4], 0x00);
}

TEST(AcseTest, DecodeAcseApduRecognizesReleaseApdus)
{
  const std::array<std::uint8_t, 2> rlrqBytes = {0x62, 0x00};
  const std::array<std::uint8_t, 2> rlreBytes = {0x63, 0x00};
  dlms::apdu::AcseApdu apdu = {};

  ASSERT_EQ(DecodeAcseApdu(rlrqBytes.data(), rlrqBytes.size(), apdu),
            ApduStatus::Ok);
  EXPECT_EQ(apdu.kind, dlms::apdu::AcseApduKind::Rlrq);

  ASSERT_EQ(DecodeAcseApdu(rlreBytes.data(), rlreBytes.size(), apdu),
            ApduStatus::Ok);
  EXPECT_EQ(apdu.kind, dlms::apdu::AcseApduKind::Rlre);
}

TEST(AcseTest, EncodeAcseApduWritesRlrq)
{
  const dlms::apdu::AcseApdu apdu = dlms::apdu::MakeRlrq();
  std::vector<std::uint8_t> output;

  ASSERT_EQ(EncodeAcseApdu(apdu, output), ApduStatus::Ok);
  ASSERT_EQ(output.size(), 2U);
  EXPECT_EQ(output[0], 0x62);
  EXPECT_EQ(output[1], 0x00);
}
