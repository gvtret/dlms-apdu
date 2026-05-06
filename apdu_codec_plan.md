# План реализации APDU codec для DLMS/COSEM

## 1. Назначение документа

Документ описывает план реализации переносимой кроссплатформенной библиотеки C++11 для кодирования и декодирования DLMS/COSEM APDU.

Библиотека является application-layer модулем и должна использоваться совместно с уже выделенными транспортными слоями:

```text
APDU codec
LLC codec
HDLC codec
WRAPPER codec
```

В первой версии реализуется codec layer без transport/session logic и без исполнения COSEM object model.

---

## 2. Зафиксированные проектные решения

| Вопрос | Решение |
|---|---|
| Ошибки | Только status-коды |
| Exceptions | Не используются в публичном API |
| Роль библиотеки | Универсальная: client + server |
| Encoding ACSE | BER для AARQ/AARE/RLRQ/RLRE |
| Encoding xDLMS | A-XDR для xDLMS APDU |
| ACSE user-information | xDLMS InitiateRequest/InitiateResponse/ConfirmedServiceError как OCTET STRING |
| Referencing v1 | LN referencing сначала |
| SN referencing | Не входит в v1, но tags не должны конфликтовать |
| Security/ciphering | Только model/tags и opaque ciphered payload в v1 |
| Block transfer | Поддержать xDLMS block APDU structures, но не state machine |
| Data model | Реализовать A-XDR Data codec как отдельный компонент |
| C ABI | Закладывается отдельным стабильным слоем |
| CMake | Минимум 3.16 |
| Tests | GoogleTest |
| C++ API buffers | Оба варианта: `std::vector` и caller-provided buffers |

---

## 3. Цель v1

Реализовать библиотеку, которая умеет:

```text
кодировать и декодировать ACSE AARQ/AARE APDU в BER
кодировать и декодировать xDLMS InitiateRequest/InitiateResponse в A-XDR
кодировать и декодировать LN GET/SET/ACTION request/response APDU
кодировать и декодировать Data и AccessResult
разбирать invoke-id-and-priority
поддерживать normal, next, block и with-list формы как структуры APDU
сохранять ciphered APDU payload как opaque bytes
работать без exceptions
возвращать ошибки через status-коды
предоставлять C++11 API
предоставлять стабильный C ABI
собираться через CMake 3.16
иметь обязательные GoogleTest-тесты и real DLMS vectors
```

---

## 4. Границы v1

### 4.1. Входит в v1

```text
BER TLV reader/writer для ACSE subset
A-XDR reader/writer для xDLMS subset
ACSE AARQ/AARE/RLRQ/RLRE codec
xDLMS InitiateRequest/InitiateResponse codec
ConfirmedServiceError codec
LN GET request/response codec
LN SET request/response codec
LN ACTION request/response codec
Data codec для базовых DLMS data types
AccessResult и DataAccessResult
InvokeIdAndPriority helpers
opaque глобально/выделенно ciphered APDU variants
C++11 API
C ABI wrapper
GoogleTest coverage
CMake 3.16
```

### 4.2. Не входит в v1

```text
COSEM object model
attribute/method execution
association state machine
authentication algorithm execution
security/ciphering AES-GCM
key management
HDLC/LLC/WRAPPER transport
retry/timeouts
SN referencing implementation
XML schema support
DLMS profile-specific business rules outside APDU encoding
```

---

## 5. Архитектурная позиция APDU codec

Для HDLC-based profile стек выглядит так:

```text
+-----------------------------+
| APDU codec                  |
+-----------------------------+
| LLC codec                   |
+-----------------------------+
| HDLC codec/session          |
+-----------------------------+
| Transport: UART/TCP/etc.    |
+-----------------------------+
```

Для WRAPPER profile:

```text
+-----------------------------+
| APDU codec                  |
+-----------------------------+
| WRAPPER codec               |
+-----------------------------+
| Transport: TCP/UDP          |
+-----------------------------+
```

APDU codec не должен знать, через какой lower layer переданы bytes. Для LLC и WRAPPER APDU является opaque payload.

---

## 6. Опора на документацию

По данным doc-rag-remote:

```text
AARQ, AARE, RLRQ и RLRE APDU кодируются BER.
user-information в AARQ содержит xDLMS InitiateRequest.
user-information в AARE содержит xDLMS InitiateResponse или confirmedServiceError.
xDLMS InitiateRequest/InitiateResponse кодируются A-XDR и вкладываются как OCTET STRING.
GET, SET и ACTION являются xDLMS services с invoke-id-and-priority.
ACTION поддерживает normal, one-block, next и block-transfer формы.
Wrapper layer переносит xDLMS APDU как Data без знания структуры APDU.
```

Практические test vectors должны быть взяты из Green Book examples и из существующих SPOdes trace кадров в интеграционных тестах.

---

## 7. Рассмотренные подходы

### 7.1. Подход A - только raw APDU tag dispatch

Плюс: минимальный объем работ.
Минус: AARQ/AARE и Data codec останутся снаружи.
Риск: каждый transport/integration тест начнет дублировать parsing logic.

### 7.2. Подход B - ACSE + xDLMS core + Data codec

Плюс: полноценная база для association и GET/SET/ACTION workflows.
Минус: больше тестов и аккуратнее model types.
Риск: можно случайно начать реализовывать COSEM application logic.

### 7.3. Подход C - codec + association/client state machine сразу

Плюс: быстрее получить demo обмен со счетчиком.
Минус: v1 раздуется.
Риск: смешаются encoding, authentication, retry, block-transfer policy и object model.

### 7.4. Выбор

Выбирается Подход B:

```text
ACSE BER codec
+ xDLMS A-XDR codec
+ Data codec
+ GET/SET/ACTION APDU model
- association/client state machine
- security execution
- COSEM object model
```

---

## 8. Структура проекта

```text
dlms-apdu/
 ├── CMakeLists.txt
 ├── include/
 │   └── dlms/
 │       └── apdu/
 │           ├── apdu_types.hpp
 │           ├── apdu_error.hpp
 │           ├── apdu_reader.hpp
 │           ├── apdu_writer.hpp
 │           ├── ber.hpp
 │           ├── axdr.hpp
 │           ├── cosem_descriptor.hpp
 │           ├── data.hpp
 │           ├── initiate.hpp
 │           ├── acse.hpp
 │           ├── get.hpp
 │           ├── set.hpp
 │           ├── action.hpp
 │           ├── xdlms.hpp
 │           └── apdu_c_api.h
 ├── src/
 │   └── apdu/
 │       ├── apdu_reader.cpp
 │       ├── apdu_writer.cpp
 │       ├── ber.cpp
 │       ├── axdr.cpp
 │       ├── data.cpp
 │       ├── initiate.cpp
 │       ├── acse.cpp
 │       ├── get.cpp
 │       ├── set.cpp
 │       ├── action.cpp
 │       ├── xdlms.cpp
 │       └── apdu_c_api.cpp
 ├── test/
 │   ├── CMakeLists.txt
 │   └── apdu/
 │       ├── test_apdu_error.cpp
 │       ├── test_ber.cpp
 │       ├── test_axdr.cpp
 │       ├── test_data.cpp
 │       ├── test_initiate.cpp
 │       ├── test_acse.cpp
 │       ├── test_get.cpp
 │       ├── test_set.cpp
 │       ├── test_action.cpp
 │       ├── test_xdlms_vectors.cpp
 │       └── test_apdu_c_api.cpp
 └── docs/
     ├── 00_apdu_requirements.md
     ├── 01_apdu_codec_api.md
     ├── 02_apdu_c_api.md
     ├── 03_acse_ber_requirements.md
     ├── 04_xdlms_axdr_requirements.md
     └── 05_apdu_test_plan.md
```

---

## 9. Базовые требования к ошибкам

### 9.1. Общий принцип

```text
Ни одна функция библиотеки не бросает исключения.
Ни одна функция библиотеки не вызывает abort/assert в runtime path.
Ошибки возвращаются только через ApduStatus.
Decoder должен отличать NeedMoreData от malformed input.
```

### 9.2. Status enum

```cpp
enum class ApduStatus
{
  Ok = 0,

  NeedMoreData,
  OutputBufferTooSmall,

  InvalidArgument,
  InvalidTag,
  InvalidLength,
  InvalidBer,
  InvalidAxdr,
  InvalidChoice,
  InvalidData,
  InvalidInvokeId,
  InvalidDescriptor,
  InvalidConformance,

  UnsupportedApdu,
  UnsupportedAcseField,
  UnsupportedXdlmsService,
  UnsupportedDataType,
  UnsupportedFeature,

  PduTooLarge,
  InternalError
};
```

---

## 10. Buffer policy

Как и в HDLC codec, используются два уровня API.

### 10.1. High-level C++ API

```cpp
ApduStatus EncodeXdlmsApdu(
  const XdlmsApdu& apdu,
  std::vector<std::uint8_t>& output);
```

Этот API удобен для application code и тестов, но может выделять память через `std::vector`.

### 10.2. Strict no-allocation API

```cpp
ApduStatus EncodeXdlmsApduToBuffer(
  const XdlmsApdu& apdu,
  std::uint8_t* output,
  std::size_t outputSize,
  std::size_t& writtenSize);
```

Этот вариант используется в C ABI и в окружениях с жесткими требованиями к памяти.

---

## 11. APDU model

### 11.1. Top-level APDU choice

```cpp
enum class ApduKind
{
  Aarq,
  Aare,
  Rlrq,
  Rlre,
  Xdlms
};
```

### 11.2. xDLMS APDU kind

```cpp
enum class XdlmsApduKind
{
  InitiateRequest,
  InitiateResponse,
  ConfirmedServiceError,
  GetRequest,
  GetResponse,
  SetRequest,
  SetResponse,
  ActionRequest,
  ActionResponse,
  DataNotification,
  ExceptionResponse,
  GeneralGloCiphering,
  GeneralDedCiphering,
  GeneralCiphering,
  UnknownCiphered
};
```

### 11.3. Lightweight byte views

```cpp
struct ByteView
{
  const std::uint8_t* data;
  std::size_t size;
};
```

Byte views используются для opaque fields: authentication-value, system-title, ciphered-content, raw-data blocks.

---

## 12. BER codec для ACSE

### 12.1. Требования

BER codec должен поддержать:

```text
definite length short form
definite length long form
constructed/context/application tags, необходимые для ACSE
OBJECT IDENTIFIER
INTEGER / ENUMERATED
BIT STRING
OCTET STRING
NULL
SEQUENCE
```

Indefinite length в v1 не поддерживать, пока нет real vector, который его требует.

### 12.2. ACSE fields v1

```text
application-context-name
sender/responder-acse-requirements
mechanism-name
calling/responding-authentication-value
result
result-source-diagnostic
user-information
```

Остальные поля: decoder может пропускать известные BER TLV как unsupported/ignored только если это явно отражено в API result metadata; encoder не должен генерировать поля, не поддержанные model.

---

## 13. A-XDR codec для xDLMS

### 13.1. Требования

A-XDR codec должен поддержать:

```text
CHOICE tag dispatch
OPTIONAL usage flag
DEFAULT usage flag
Unsigned8/Unsigned16/Unsigned32
Integer8/Integer16/Integer32
BOOLEAN
ENUMERATED
OCTET STRING with A-XDR length
BIT STRING Conformance with application tag 0x5f1f
SEQUENCE fixed-order encoding
```

### 13.2. InitiateRequest

```text
tag: 0x01
dedicated-key OPTIONAL
response-allowed DEFAULT TRUE
proposed-quality-of-service OPTIONAL
proposed-dlms-version-number
proposed-conformance
client-max-receive-pdu-size
```

### 13.3. InitiateResponse

```text
tag: 0x08
negotiated-quality-of-service OPTIONAL
negotiated-dlms-version-number
negotiated-conformance
server-max-receive-pdu-size
vaa-name
```

---

## 14. Conformance codec

### 14.1. Требования

Реализовать typed bitset для 24-bit conformance.

Минимально нужны флаги:

```text
general-protection
general-block-transfer
read
write
unconfirmed-write
attribute0-supported-with-set
priority-mgmt-supported
attribute0-supported-with-get
block-transfer-with-get-or-read
block-transfer-with-set-or-write
block-transfer-with-action
multiple-references
information-report
parameterized-access
get
set
selective-access
event-notification
action
```

### 14.2. Вектор из документации

Для LN referencing должен быть тест с `00 7E 1F`, так как doc-rag-remote возвращает этот conformance value в InitiateRequest examples.

---

## 15. Data codec

### 15.1. Входит в v1

```text
null-data
array
structure
boolean
bit-string
double-long
double-long-unsigned
octet-string
visible-string
utf8-string
bcd
integer
long
unsigned
long-unsigned
compact-array как unsupported tag с корректной ошибкой
long64
long64-unsigned
enum
float32
float64
date-time
date
time
dont-care
```

### 15.2. Ограничение

Data codec только кодирует/декодирует typed values. Он не интерпретирует OBIS, scaling, units или class-specific semantics.

---

## 16. GET codec

### 16.1. Входит в v1

```text
GetRequestNormal
GetRequestNext
GetRequestWithList
GetResponseNormal
GetResponseWithDataBlock
GetResponseWithList
```

### 16.2. Критичные проверки

```text
invoke-id-and-priority сохраняется roundtrip
class-id кодируется big-endian Unsigned16
logical-name ровно 6 bytes
attribute-id один byte
selective-access optional flag обрабатывается явно
data block last-block/block-number/raw-data сохраняется
```

---

## 17. SET codec

### 17.1. Входит в v1

```text
SetRequestNormal
SetRequestFirstDataBlock
SetRequestWithDataBlock
SetRequestWithList
SetRequestWithListAndFirstDataBlock
SetResponseNormal
SetResponseDataBlock
SetResponseLastDataBlock
SetResponseWithList
```

### 17.2. Граница ответственности

Codec не решает, когда резать payload на blocks. Он только кодирует структуру APDU, которую ему передали.

---

## 18. ACTION codec

### 18.1. Входит в v1

```text
ActionRequestNormal
ActionRequestNextPblock
ActionRequestWithList
ActionRequestWithFirstPblock
ActionRequestWithListAndFirstPblock
ActionRequestWithPblock
ActionResponseNormal
ActionResponseWithPblock
ActionResponseWithList
ActionResponseNextPblock
```

### 18.2. Граница ответственности

Codec не вызывает методы COSEM объектов и не управляет двухфазным block transfer. Он только сохраняет request/response type, block-number, raw-data и result fields.

---

## 19. Ciphered APDU handling

В v1 ciphered APDU не расшифровываются.

Decoder должен:

```text
распознать glo/ded/general ciphering tags
вернуть kind + opaque bytes
не пытаться парсить ciphered-content
не валидировать authentication tag
```

Encoder должен:

```text
уметь вернуть тот же tag и opaque bytes
не требовать ключей
```

Реальная security implementation относится к отдельному будущему модулю.

---

## 20. Limits

```cpp
struct ApduCodecLimits
{
  std::size_t maximumApduSize;
  std::size_t maximumBerDepth;
  std::size_t maximumAxdrDepth;
  std::size_t maximumArrayElements;
  std::size_t maximumStructureElements;
  std::size_t maximumOctetStringSize;
  std::size_t maximumRawDataBlockSize;
};
```

Limits должны применяться и к high-level API, и к caller-provided buffer API.

---

## 21. C ABI

### 21.1. Требования

Файлы:

```text
include/dlms/apdu/apdu_c_api.h
src/apdu/apdu_c_api.cpp
```

### 21.2. Принципы C ABI

```text
extern "C"
никаких C++ типов в ABI
никаких exceptions
только fixed integer types
caller-provided buffers
стабильные enum values
```

### 21.3. Минимальный C API

```c
dlms_apdu_status_t dlms_apdu_decode_xdlms(
  const uint8_t* input,
  size_t input_size,
  dlms_apdu_xdlms_t* output);
```

```c
dlms_apdu_status_t dlms_apdu_encode_xdlms(
  const dlms_apdu_xdlms_t* input,
  uint8_t* output,
  size_t output_size,
  size_t* written_size);
```

Для complex Data values C ABI может сначала предоставить raw view API и только затем typed tree handles, чтобы не раздуть v1.

---

## 22. CMake

```cmake
cmake_minimum_required(VERSION 3.16)

project(dlms_apdu
  VERSION 0.1.0
  LANGUAGES C CXX)

option(DLMS_BUILD_TESTS "Build tests" ON)
option(DLMS_BUILD_C_API "Build C ABI wrapper" ON)
option(DLMS_USE_SYSTEM_GTEST "Use system GoogleTest" OFF)

add_library(dlms_apdu
  src/apdu/apdu_reader.cpp
  src/apdu/apdu_writer.cpp
  src/apdu/ber.cpp
  src/apdu/axdr.cpp
  src/apdu/data.cpp
  src/apdu/initiate.cpp
  src/apdu/acse.cpp
  src/apdu/get.cpp
  src/apdu/set.cpp
  src/apdu/action.cpp
  src/apdu/xdlms.cpp)

target_compile_features(dlms_apdu PUBLIC cxx_std_11)

target_include_directories(dlms_apdu
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(DLMS_APDU_HAS_CODEC_API ON CACHE BOOL "dlms-apdu codec API is available" FORCE)

if(DLMS_BUILD_C_API)
  target_sources(dlms_apdu
    PRIVATE
      src/apdu/apdu_c_api.cpp)
endif()

if(DLMS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
```

---

## 23. GoogleTest integration

Базовый вариант повторяет HDLC:

```text
по умолчанию FetchContent
при DLMS_USE_SYSTEM_GTEST=ON использовать системный GoogleTest
```

Для offline-среды можно позже добавить vendored-режим.

---

## 24. Тестовая стратегия

### 24.1. BER tests

```text
DecodeBer_shortLength
DecodeBer_longLength
DecodeBer_nestedSequence
DecodeBer_objectIdentifier
DecodeBer_rejectsIndefiniteLength
EncodeBer_roundtrip
DecodeBer_depthLimit
```

### 24.2. A-XDR tests

```text
DecodeAxdr_optionalAbsent
DecodeAxdr_optionalPresent
DecodeAxdr_defaultAbsent
DecodeAxdr_unsignedValues
DecodeAxdr_octetString
DecodeAxdr_conformanceApplicationTag
EncodeAxdr_roundtrip
```

### 24.3. Initiate tests

```text
DecodeInitiateRequest_greenBookLnVector
EncodeInitiateRequest_greenBookLnVector
DecodeInitiateResponse_greenBookVector
EncodeInitiateResponse_greenBookVector
RejectInitiateRequest_missingConformance
RejectInitiateResponse_missingVaaName
```

### 24.4. ACSE tests

```text
DecodeAarq_withUserInformation
EncodeAarq_roundtrip
DecodeAare_acceptWithInitiateResponse
DecodeAare_rejectWithConfirmedServiceError
DecodeAarq_withAuthenticationValue
RejectAarq_invalidUserInformationOctetString
```

### 24.5. Data tests

```text
DecodeData_null
DecodeData_boolean
DecodeData_unsigned
DecodeData_octetString
DecodeData_visibleString
DecodeData_dateTime
DecodeData_array
DecodeData_structure
RejectData_unknownTag
RejectData_depthLimit
```

### 24.6. GET tests

```text
DecodeGetRequestNormal_spodesVector
EncodeGetRequestNormal_spodesVector
DecodeGetResponseNormal_spodesVector
EncodeGetResponseNormal_spodesVector
DecodeGetResponseWithDataBlock
DecodeGetRequestWithList
RejectGetRequest_truncatedDescriptor
```

### 24.7. SET tests

```text
EncodeSetRequestNormal_roundtrip
DecodeSetResponseNormal
EncodeSetRequestWithFirstDataBlock
DecodeSetResponseLastDataBlock
EncodeSetRequestWithList_roundtrip
```

### 24.8. ACTION tests

```text
EncodeActionRequestNormal_roundtrip
DecodeActionResponseNormal
EncodeActionRequestNextPblock
DecodeActionResponseWithPblock
EncodeActionRequestWithList_roundtrip
```

### 24.9. C API tests

```text
CApi_decodeXdlms
CApi_encodeXdlms
CApi_outputBufferTooSmall
CApi_noCrashOnNullArguments
CApi_cHeaderCompilesAsC
```

### 24.10. Cross-layer integration tests

В корневом `E:\work\dlms\test\integration` должны быть сценарии:

```text
APDU -> LLC -> HDLC -> LLC -> APDU для AARQ
APDU -> LLC -> HDLC -> LLC -> APDU для GET request
APDU -> WRAPPER -> APDU для AARQ
APDU -> WRAPPER -> APDU для GET request
SPOdes trace HDLC frame -> LLC -> APDU для AARQ/AARE/GET
payload byte 0x7E внутри APDU сохраняется lower layers
```

---

## 25. Реализационные фазы

### Фаза 0. Документы требований

Результат:

```text
docs/00_apdu_requirements.md
docs/01_apdu_codec_api.md
docs/02_apdu_c_api.md
docs/03_acse_ber_requirements.md
docs/04_xdlms_axdr_requirements.md
docs/05_apdu_test_plan.md
```

Критерий готовности:

```text
ACSE/xDLMS/Data scope явно зафиксирован
границы transport/session/security не смешаны с codec
```

Commit message:

```text
docs(apdu): define codec requirements and API boundaries
```

---

### Фаза 1. Каркас проекта

Результат:

```text
CMakeLists.txt
include/dlms/apdu/*.hpp
src/apdu/*.cpp
test/CMakeLists.txt
```

Критерий готовности:

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

Commit message:

```text
build(apdu): add project structure and test harness
```

---

### Фаза 2. Status/error model и базовые reader/writer

Результат:

```text
apdu_error.hpp
apdu_reader.hpp/.cpp
apdu_writer.hpp/.cpp
```

Критерий готовности:

```text
единый ApduStatus
NeedMoreData и malformed input различаются
caller-provided buffers работают без allocation
```

Commit message:

```text
feat(apdu): add status model and byte reader writer
```

---

### Фаза 3. BER codec

Результат:

```text
ber.hpp
ber.cpp
test_ber.cpp
```

Критерий готовности:

```text
BER definite length TLV roundtrip проходит
ACSE-required primitive tags поддержаны
indefinite length отклоняется понятным status
```

Commit message:

```text
feat(apdu): implement BER TLV codec for ACSE subset
```

---

### Фаза 4. A-XDR codec

Результат:

```text
axdr.hpp
axdr.cpp
test_axdr.cpp
```

Критерий готовности:

```text
OPTIONAL/DEFAULT flags проходят
Unsigned/Integer/Boolean/OCTET STRING проходят
Conformance application tag 0x5f1f проходит
```

Commit message:

```text
feat(apdu): implement A-XDR primitives for xDLMS
```

---

### Фаза 5. Conformance и Initiate APDU

Результат:

```text
initiate.hpp
initiate.cpp
test_initiate.cpp
```

Критерий готовности:

```text
Green Book InitiateRequest vector проходит
InitiateResponse roundtrip проходит
client/server max PDU size сохраняется
```

Commit message:

```text
feat(apdu): add xDLMS initiate request response codec
```

---

### Фаза 6. ACSE AARQ/AARE/RLRQ/RLRE

Результат:

```text
acse.hpp
acse.cpp
test_acse.cpp
```

Критерий готовности:

```text
AARQ содержит InitiateRequest в user-information OCTET STRING
AARE содержит InitiateResponse или ConfirmedServiceError
authentication-related fields сохраняются как model fields
```

Commit message:

```text
feat(apdu): implement ACSE association APDU codec
```

---

### Фаза 7. Data codec

Результат:

```text
data.hpp
data.cpp
test_data.cpp
```

Критерий готовности:

```text
basic scalar Data types проходят
array/structure recursion проходит
depth и size limits соблюдаются
```

Commit message:

```text
feat(apdu): implement DLMS Data codec
```

---

### Фаза 8. GET codec

Результат:

```text
get.hpp
get.cpp
test_get.cpp
```

Критерий готовности:

```text
normal/next/with-list requests проходят
normal/data-block/with-list responses проходят
SPOdes GET request/response vectors декодируются
```

Commit message:

```text
feat(apdu): implement LN GET APDU codec
```

---

### Фаза 9. SET codec

Результат:

```text
set.hpp
set.cpp
test_set.cpp
```

Критерий готовности:

```text
normal/block/with-list SET forms проходят
DataAccessResult сохраняется
block raw-data не интерпретируется codec layer
```

Commit message:

```text
feat(apdu): implement LN SET APDU codec
```

---

### Фаза 10. ACTION codec

Результат:

```text
action.hpp
action.cpp
test_action.cpp
```

Критерий готовности:

```text
normal/next/block/with-list ACTION forms проходят
method descriptors кодируются корректно
result + optional response parameters сохраняются
```

Commit message:

```text
feat(apdu): implement LN ACTION APDU codec
```

---

### Фаза 11. Top-level xDLMS dispatch и ciphered opaque variants

Результат:

```text
xdlms.hpp
xdlms.cpp
test_xdlms_vectors.cpp
```

Критерий готовности:

```text
top-level tags dispatch работают
unsupported known tags возвращают UnsupportedXdlmsService
ciphered APDU возвращают opaque payload без расшифровки
```

Commit message:

```text
feat(apdu): add top-level xDLMS dispatch
```

---

### Фаза 12. C ABI

Результат:

```text
apdu_c_api.h
apdu_c_api.cpp
test_apdu_c_api.cpp
```

Критерий готовности:

```text
C header компилируется C-компилятором
C API не содержит C++ типов
null arguments возвращают InvalidArgument
output buffer too small возвращается предсказуемо
```

Commit message:

```text
feat(apdu): expose stable C ABI
```

---

### Фаза 13. Интеграционные тесты root проекта

Результат:

```text
E:/work/dlms/test/integration/test_apdu_stack_integration.cpp
E:/work/dlms/test/integration/CMakeLists.txt
E:/work/dlms/CMakeLists.txt
```

Критерий готовности:

```text
APDU roundtrip через LLC+HDLC проходит
APDU roundtrip через WRAPPER проходит, если dlms-wrapper codec API доступен
SPOdes trace frames декодируются до typed APDU
интеграционный target не собирается до появления DLMS_APDU_HAS_CODEC_API
```

Commit message:

```text
test(integration): add APDU cross-layer codec vectors
```

---

### Фаза 14. Doxygen public API documentation

Критерий готовности:

```text
каждая функция публичного API документирована
ownership и lifetime для ByteView описаны
limits и error statuses описаны
C ABI header самодокументируемый
```

Commit message:

```text
docs(apdu): document public codec API
```

---

## 26. Основные риски

### 26.1. Смешивание BER и A-XDR

Контроль:

```text
ACSE только через ber.*
xDLMS только через axdr.*
user-information bridge тестируется отдельно
```

### 26.2. Слишком ранняя association state machine

Контроль:

```text
codec не решает authentication challenge
codec не выбирает conformance
codec не управляет release/open state
```

### 26.3. Неполная Data модель

Контроль:

```text
unsupported tags возвращают явный status
тесты фиксируют каждый поддержанный data tag
compact-array не маскируется под raw bytes
```

### 26.4. Ошибки block transfer semantics

Контроль:

```text
codec только кодирует APDU forms
block sequencing и запрос next block относятся к future client/session layer
```

### 26.5. Несовместимость с реальными счетчиками

Контроль:

```text
Green Book vectors
SPOdes trace vectors
negative tests with one-byte corruption/truncation
cross-layer tests через LLC+HDLC и WRAPPER
```

---

## 27. Итоговый milestone v1

```text
M1: Portable C++11 DLMS/COSEM APDU Codec
```

Состав:

```text
CMake 3.16
C++11
no exceptions in public API
status-code API
BER ACSE AARQ/AARE/RLRQ/RLRE codec
A-XDR xDLMS codec
InitiateRequest/InitiateResponse
ConfirmedServiceError
LN GET/SET/ACTION APDU
Data codec
Conformance codec
opaque ciphered APDU support
configurable limits
stable C ABI
GoogleTest
real DLMS vectors
root cross-layer integration tests
Doxygen-documented public API
```

Не входит:

```text
COSEM object model
association state machine
security/ciphering execution
transport
retry/timeouts
SN referencing implementation
```

---

## 28. Следующий практический шаг

Начинать реализацию следует с документов и каркаса:

```text
1. docs/00_apdu_requirements.md
2. docs/01_apdu_codec_api.md
3. docs/02_apdu_c_api.md
4. docs/03_acse_ber_requirements.md
5. docs/04_xdlms_axdr_requirements.md
6. docs/05_apdu_test_plan.md
7. CMake project structure
8. empty library target
9. GoogleTest harness
10. ApduStatus + reader/writer
```

Только после этого переходить к BER и A-XDR codec.
