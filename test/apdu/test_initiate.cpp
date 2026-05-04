#include "dlms/apdu/initiate.hpp"

#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

namespace {

using dlms::apdu::ApduStatus;
using dlms::apdu::ApduWriter;
using dlms::apdu::DecodeInitiateRequest;
using dlms::apdu::DecodeInitiateResponse;
using dlms::apdu::EncodeInitiateRequest;
using dlms::apdu::EncodeInitiateResponse;
using dlms::apdu::InitiateRequest;
using dlms::apdu::InitiateResponse;
using dlms::apdu::MakeDefaultInitiateRequest;

const std::uint8_t kHdlcTraceInitiateRequest[] = {
  0x01, 0x00, 0x00, 0x00, 0x06, 0x5f, 0x1f, 0x04,
  0x00, 0x62, 0x1e, 0x5d, 0x02, 0x00
};

const std::uint8_t kHdlcTraceInitiateResponse[] = {
  0x08, 0x00, 0x06, 0x5f, 0x1f, 0x04, 0x00, 0x40,
  0x18, 0x1d, 0x02, 0x00, 0x00, 0x07
};

TEST(InitiateCodecTest, DecodeSpodesHdlcTraceInitiateRequest)
{
  InitiateRequest request = {};
  ASSERT_EQ(ApduStatus::Ok,
            DecodeInitiateRequest(kHdlcTraceInitiateRequest,
                                  sizeof(kHdlcTraceInitiateRequest),
                                  request));

  EXPECT_FALSE(request.hasDedicatedKey);
  EXPECT_TRUE(request.responseAllowed);
  EXPECT_FALSE(request.hasProposedQualityOfService);
  EXPECT_EQ(6u, request.proposedDlmsVersionNumber);
  EXPECT_EQ(0x62u, request.proposedConformance.bytes[0]);
  EXPECT_EQ(0x1eu, request.proposedConformance.bytes[1]);
  EXPECT_EQ(0x5du, request.proposedConformance.bytes[2]);
  EXPECT_EQ(0x0200u, request.clientMaxReceivePduSize);
}

TEST(InitiateCodecTest, EncodeSpodesHdlcTraceInitiateRequest)
{
  InitiateRequest request = {};
  ASSERT_EQ(ApduStatus::Ok,
            DecodeInitiateRequest(kHdlcTraceInitiateRequest,
                                  sizeof(kHdlcTraceInitiateRequest),
                                  request));

  std::uint8_t output[sizeof(kHdlcTraceInitiateRequest)] = {};
  ApduWriter writer(output, sizeof(output));
  ASSERT_EQ(ApduStatus::Ok, EncodeInitiateRequest(request, writer));
  ASSERT_EQ(sizeof(kHdlcTraceInitiateRequest), writer.WrittenSize());
  for (std::size_t i = 0; i < sizeof(kHdlcTraceInitiateRequest); ++i) {
    EXPECT_EQ(kHdlcTraceInitiateRequest[i], output[i]);
  }
}

TEST(InitiateCodecTest, DecodeSpodesHdlcTraceInitiateResponse)
{
  InitiateResponse response = {};
  ASSERT_EQ(ApduStatus::Ok,
            DecodeInitiateResponse(kHdlcTraceInitiateResponse,
                                   sizeof(kHdlcTraceInitiateResponse),
                                   response));

  EXPECT_FALSE(response.hasNegotiatedQualityOfService);
  EXPECT_EQ(6u, response.negotiatedDlmsVersionNumber);
  EXPECT_EQ(0x40u, response.negotiatedConformance.bytes[0]);
  EXPECT_EQ(0x18u, response.negotiatedConformance.bytes[1]);
  EXPECT_EQ(0x1du, response.negotiatedConformance.bytes[2]);
  EXPECT_EQ(0x0200u, response.serverMaxReceivePduSize);
  EXPECT_EQ(0x0007u, response.vaaName);
}

TEST(InitiateCodecTest, EncodeSpodesHdlcTraceInitiateResponse)
{
  InitiateResponse response = {};
  ASSERT_EQ(ApduStatus::Ok,
            DecodeInitiateResponse(kHdlcTraceInitiateResponse,
                                   sizeof(kHdlcTraceInitiateResponse),
                                   response));

  std::uint8_t output[sizeof(kHdlcTraceInitiateResponse)] = {};
  ApduWriter writer(output, sizeof(output));
  ASSERT_EQ(ApduStatus::Ok, EncodeInitiateResponse(response, writer));
  ASSERT_EQ(sizeof(kHdlcTraceInitiateResponse), writer.WrittenSize());
  for (std::size_t i = 0; i < sizeof(kHdlcTraceInitiateResponse); ++i) {
    EXPECT_EQ(kHdlcTraceInitiateResponse[i], output[i]);
  }
}

TEST(InitiateCodecTest, DefaultInitiateRequestUsesDocumentedLnConformance)
{
  const InitiateRequest request = MakeDefaultInitiateRequest();

  EXPECT_FALSE(request.hasDedicatedKey);
  EXPECT_TRUE(request.responseAllowed);
  EXPECT_FALSE(request.hasProposedQualityOfService);
  EXPECT_EQ(6u, request.proposedDlmsVersionNumber);
  EXPECT_EQ(0x00u, request.proposedConformance.bytes[0]);
  EXPECT_EQ(0x7eu, request.proposedConformance.bytes[1]);
  EXPECT_EQ(0x1fu, request.proposedConformance.bytes[2]);
  EXPECT_EQ(0x0200u, request.clientMaxReceivePduSize);
}

TEST(InitiateCodecTest, RejectsWrongInitiateRequestTag)
{
  const std::uint8_t bytes[] = {0x08};
  InitiateRequest request = {};

  EXPECT_EQ(ApduStatus::InvalidTag,
            DecodeInitiateRequest(bytes, sizeof(bytes), request));
}

TEST(InitiateCodecTest, RejectsTruncatedInitiateResponse)
{
  InitiateResponse response = {};

  EXPECT_EQ(ApduStatus::NeedMoreData,
            DecodeInitiateResponse(kHdlcTraceInitiateResponse,
                                   sizeof(kHdlcTraceInitiateResponse) - 1,
                                   response));
}

TEST(InitiateCodecTest, EncodesResponseAllowedFalse)
{
  InitiateRequest request = MakeDefaultInitiateRequest();
  request.responseAllowed = false;

  std::uint8_t output[16] = {};
  ApduWriter writer(output, sizeof(output));
  ASSERT_EQ(ApduStatus::Ok, EncodeInitiateRequest(request, writer));

  EXPECT_EQ(0x01u, output[0]);
  EXPECT_EQ(0x00u, output[1]);
  EXPECT_EQ(0x01u, output[2]);
  EXPECT_EQ(0x00u, output[3]);
}

} // namespace
