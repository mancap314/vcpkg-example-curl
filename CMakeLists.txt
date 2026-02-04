cmake_minimum_required(VERSION 3.14...4.1)

project(trycurl)

find_package(CURL REQUIRED)

add_executable(curl_zero src/main.c)

set_property(TARGET curl_zero PROPERTY C_STANDARD 11)
set_property(TARGET curl_zero PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET curl_zero PROPERTY C_EXTENSIONS OFF)

target_link_libraries(curl_zero PRIVATE CURL::libcurl)
