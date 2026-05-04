cmake_minimum_required(VERSION 3.16)

project(dlms_apdu
  VERSION 0.1.0
  LANGUAGES C CXX)

option(DLMS_BUILD_TESTS "Build GoogleTest tests" ON)
option(DLMS_BUILD_C_API "Build stable C ABI wrapper" ON)
option(DLMS_USE_SYSTEM_GTEST "Use system-installed GoogleTest" OFF)

add_library(dlms_apdu
  src/apdu/apdu_error.cpp
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

if(DLMS_BUILD_C_API)
  target_sources(dlms_apdu
    PRIVATE
      src/apdu/apdu_c_api.cpp)
endif()

target_compile_features(dlms_apdu PUBLIC cxx_std_11)

target_include_directories(dlms_apdu
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(DLMS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
