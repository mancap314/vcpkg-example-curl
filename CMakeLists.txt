cmake_minimum_required(VERSION 3.14...4.1)

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

project(trycurl)

find_package(CURL REQUIRED)

add_executable(curl_zero src/main.c src/utils.c)

target_include_directories(curl_zero PRIVATE include)

set_property(TARGET curl_zero PROPERTY C_STANDARD 11)
set_property(TARGET curl_zero PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET curl_zero PROPERTY C_EXTENSIONS OFF)

# if(BUILD_SHARED_LIBS)
#   # Dynamic build - link with DLL import library
#   find_library(LEXBOR_LIBRARY NAMES lexbor REQUIRED)
# else()
#   # Static build - link with static library
#   target_compile_definitions(curl_zero PRIVATE LEXBOR_STATIC)
#   find_library(LEXBOR_LIBRARY NAMES lexbor REQUIRED)
# endif()

find_path(LEXBOR_INCLUDE_DIR NAMES lexbor/core/base.h REQUIRED)
if (BUILD_SHARED_LIBS)
  find_library(LEXBOR_LIBRARY NAMES lexbor REQUIRED)
else()
  target_compile_definitions(curl_zero PRIVATE LEXBOR_STATIC)
  find_library(LEXBOR_LIBRARY NAMES lexbor_static REQUIRED)
endif()

target_link_libraries(curl_zero PRIVATE CURL::libcurl)

target_include_directories(curl_zero PRIVATE ${LEXBOR_INCLUDE_DIR})
target_link_libraries(curl_zero PRIVATE ${LEXBOR_LIBRARY})

