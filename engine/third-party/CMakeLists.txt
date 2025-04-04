cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)
project(MyProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THIRD_PARTY_INSTALL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../build_deps/_install)

include(../cmake/nlohmann-json.cmake)
include(../cmake/c-ares.cmake)
include(../cmake/jsoncpp.cmake)
include(../cmake/openssl.cmake)