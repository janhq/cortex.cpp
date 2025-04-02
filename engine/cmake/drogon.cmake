include(FetchContent)

set(USE_OSSP_UUID TRUE)

set(OPENSSL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/openssl)
set(OPENSSL_INCLUDE_DIR ${OPENSSL_INSTALL_DIR}/include)
set(OPENSSL_ROOT_DIR ${OPENSSL_INSTALL_DIR}/lib)

set(ZLIB_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/zlib/install)
set(ZLIB_INCLUDE_DIR ${ZLIB_INSTALL_DIR}/include)
set(ZLIB_LIBRARY ${ZLIB_INSTALL_DIR}/lib/zlib.lib)


set(JSONCPP_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/jsoncpp-lib)
set(JSONCPP_LIBRARIES ${JSONCPP_INSTALL_DIR}/lib/jsoncpp.lib)
set(JSONCPP_INCLUDE_DIRS ${JSONCPP_INSTALL_DIR}/include)

set(C-ARES_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/c-ares-lib)
set(C-ARES_LIBRARIES ${C-ARES_INSTALL_DIR}/lib/cares.lib)
set(C-ARES_INCLUDE_DIRS ${C-ARES_INSTALL_DIR}/include)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
FetchContent_Declare(drogon
    GIT_REPOSITORY https://github.com/drogonframework/drogon.git
    GIT_TAG v1.9.10
)

FetchContent_MakeAvailableWithArgs(drogon BUILD_CTL=OFF)
