# dlms-apdu

`dlms-apdu` implements portable C++11 codecs for DLMS/COSEM
application-layer APDUs.

The library covers ACSE BER APDUs, xDLMS A-XDR APDUs, selected LN
GET/SET/ACTION services, and DLMS Data values used by those services. It does
not implement transport profiles, association orchestration, COSEM object
storage, or cryptographic execution.

## Scope

Included:

- BER helper codec;
- A-XDR helper codec;
- ACSE AARQ/AARE APDU handling;
- xDLMS InitiateRequest and InitiateResponse;
- LN GET/SET/ACTION request and response structures;
- DLMS Data values required by supported services;
- opaque ciphered APDU variants;
- stable C ABI wrapper;
- GoogleTest unit tests.

Not included:

- HDLC, LLC, Wrapper, or transport headers;
- Application Association state machine;
- AES-GCM ciphering or HLS authentication;
- key management;
- COSEM object model;
- SN referencing behavior.

## Documentation

- [requirements](docs/00_apdu_requirements.md)
- [codec API](docs/01_apdu_codec_api.md)
- [C API](docs/02_apdu_c_api.md)
- [ACSE BER requirements](docs/03_acse_ber_requirements.md)
- [xDLMS A-XDR requirements](docs/04_xdlms_axdr_requirements.md)
- [test plan](docs/05_apdu_test_plan.md)
- [public API reference](docs/06_public_api_reference.md)
- [architecture](docs/architecture.md)
