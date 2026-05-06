#ifndef DLMS_APDU_ACSE_HPP
#define DLMS_APDU_ACSE_HPP

#include "dlms/apdu/apdu_error.hpp"
#include "dlms/apdu/apdu_types.hpp"
#include "dlms/apdu/apdu_writer.hpp"
#include "dlms/apdu/initiate.hpp"

#include <cstdint>
#include <vector>

namespace dlms {
namespace apdu {

struct AcseRawField
{
  std::uint8_t tag;
  ByteView encoded;
};

struct AarqApdu
{
  std::vector<AcseRawField> fields;
  InitiateRequest initiateRequest;
};

struct AareApdu
{
  std::vector<AcseRawField> fields;
  bool hasResult;
  std::int32_t result;
  bool hasDiagnostic;
  std::int32_t diagnostic;
  InitiateResponse initiateResponse;
};

struct RlrqApdu
{
  std::vector<AcseRawField> fields;
  bool hasReason;
  std::int32_t reason;
};

struct RlreApdu
{
  std::vector<AcseRawField> fields;
  bool hasReason;
  std::int32_t reason;
};

struct XdlmsApdu;

enum class AcseApduKind
{
  Aarq,
  Aare,
  Rlrq,
  Rlre
};

struct AcseApdu
{
  AcseApduKind kind;
  AarqApdu aarq;
  AareApdu aare;
  RlrqApdu rlrq;
  RlreApdu rlre;
};

ApduStatus DecodeAarq(
  const std::uint8_t* input,
  std::size_t inputSize,
  AarqApdu& output);

ApduStatus EncodeAarq(
  const AarqApdu& input,
  ApduWriter& writer);

ApduStatus DecodeAare(
  const std::uint8_t* input,
  std::size_t inputSize,
  AareApdu& output);

ApduStatus EncodeAare(
  const AareApdu& input,
  ApduWriter& writer);

ApduStatus DecodeRlrq(
  const std::uint8_t* input,
  std::size_t inputSize,
  RlrqApdu& output);

ApduStatus EncodeRlrq(
  const RlrqApdu& input,
  ApduWriter& writer);

ApduStatus DecodeRlre(
  const std::uint8_t* input,
  std::size_t inputSize,
  RlreApdu& output);

ApduStatus EncodeRlre(
  const RlreApdu& input,
  ApduWriter& writer);

AcseApdu MakeAarqWithInitiateRequest(const XdlmsApdu& initiateRequest);

AcseApdu MakeRlrq();

ApduStatus DecodeAcseApdu(
  const std::uint8_t* input,
  std::size_t inputSize,
  AcseApdu& output);

ApduStatus EncodeAcseApdu(
  const AcseApdu& input,
  std::vector<std::uint8_t>& output);

} // namespace apdu
} // namespace dlms

#endif // DLMS_APDU_ACSE_HPP
