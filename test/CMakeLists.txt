if(DLMS_USE_SYSTEM_GTEST)
  find_package(GTest REQUIRED)
elseif(NOT TARGET GTest::gtest_main)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(googletest)
endif()

add_executable(dlms_apdu_tests
  apdu/test_apdu_error.cpp
  apdu/test_apdu_reader.cpp
  apdu/test_apdu_writer.cpp
  apdu/test_apdu_skeleton.cpp)

target_link_libraries(dlms_apdu_tests
  PRIVATE
    dlms_apdu
    GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(dlms_apdu_tests)
