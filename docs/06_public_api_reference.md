# Public API Reference Notes

This file is the Doxygen-oriented public API checklist for `include/dlms/apdu`.
It documents the ownership and error contracts that apply to every public
function in the APDU codec.

## Global Contracts

All public functions return `ApduStatus` unless they are simple constructors or
factory helpers.

Decoder inputs use `(const std::uint8_t* input, std::size_t inputSize)`.
`input == nullptr` is invalid when `inputSize != 0`.

Writer-based encoders append to the supplied `ApduWriter` and return
`OutputBufferTooSmall` when the writer cannot accept the full encoded APDU.

Vector-based encoders write a complete owning `std::vector<std::uint8_t>`.

`ByteView` and C ABI payload pointers are non-owning views. They never transfer
ownership.

## Public Headers

### `apdu_error.hpp`

- `ApduStatusName` returns a static string for a status value.

### `apdu_reader.hpp` / `apdu_writer.hpp`

- Reader functions consume bytes from a caller-owned immutable input buffer.
- Writer functions copy bytes into a caller-owned mutable output buffer.

### `ber.hpp`

- BER helpers read and write definite-length BER TLV values.
- Indefinite length is rejected with an error status.

### `axdr.hpp`

- A-XDR helpers read and write booleans, optional flags, lengths, octet strings
  and conformance bit strings.
- Returned octet strings point into the reader input buffer.

### `data.hpp`

- `DecodeDlmsData` decodes one complete DATA value and rejects trailing bytes.
- `DecodeDlmsDataFromReader` consumes one DATA value from an existing reader.
- `EncodeDlmsData` writes one DATA value.

### `acse.hpp`

- ACSE functions encode and decode BER AARQ/AARE and top-level ACSE APDUs.
- ACSE user-information embeds or extracts xDLMS payload bytes.

### `get.hpp`, `set.hpp`, `action.hpp`

- Normal APIs support simple callers.
- Generic APIs support all request/response choices implemented in v1,
  including selective access, block transfer APDU structures and list forms.
- Block sequencing policy is not implemented here; this layer only preserves
  APDU fields.

### `xdlms.hpp`

- `DecodeXdlmsApdu` dispatches by top-level xDLMS tag.
- `EncodeXdlmsApdu` encodes the selected APDU model.
- Ciphered APDUs are opaque and are not decrypted.

### `apdu_c_api.h`

- C ABI functions use fixed C enums, fixed integer types and caller-provided
  buffers.
- The C ABI exposes raw xDLMS APDU views and never exposes C++ containers.
