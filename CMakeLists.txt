cmake_minimum_required(VERSION 3.14...4.1)

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

project(trycurl)

find_package(CURL REQUIRED)

add_executable(curl_zero src/main.c src/utils.c)

find_path(LEXBOR_INCLUDE_DIR NAMES lexbor/core/base.h REQUIRED)
if (BUILD_SHARED_LIBS)
  find_library(LEXBOR_LIBRARY NAMES lexbor REQUIRED)
else()
  target_compile_definitions(curl_zero PRIVATE LEXBOR_STATIC)
  find_library(LEXBOR_LIBRARY NAMES lexbor_static REQUIRED)
endif()

target_include_directories(curl_zero PRIVATE include)

set_property(TARGET curl_zero PROPERTY C_STANDARD 11)
set_property(TARGET curl_zero PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET curl_zero PROPERTY C_EXTENSIONS OFF)

target_link_libraries(curl_zero PRIVATE CURL::libcurl)

target_include_directories(curl_zero PRIVATE ${LEXBOR_INCLUDE_DIR})
target_link_libraries(curl_zero PRIVATE ${LEXBOR_LIBRARY})
target_compile_definitions(curl_zero PRIVATE
    CACERT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/ca-certificates/cacert.pem"
)

option(CURL_TITLE_BUILD_TESTS "Build tests for utils library" ON)

if (CURL_TITLE_BUILD_TESTS)
  enable_testing()

  find_package(check CONFIG REQUIRED)

  add_executable(test_utils tests/test_utils.c src/utils.c)

  target_include_directories(test_utils PRIVATE include)

  target_compile_definitions(test_utils PRIVATE LEXBOR_STATIC)
  target_include_directories(test_utils PRIVATE ${LEXBOR_INCLUDE_DIR})
  target_link_libraries(test_utils PRIVATE ${LEXBOR_LIBRARY})

  target_link_libraries(test_utils PRIVATE Check::check)
  target_link_libraries(test_utils PRIVATE CURL::libcurl)

  target_compile_definitions(test_utils PRIVATE
      CACERT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/ca-certificates/cacert.pem"
  )
  # Register with CTest
  add_test(NAME test_basic COMMAND test_utils)
  set_tests_properties(test_basic PROPERTIES
      TIMEOUT 30
  )
endif()
