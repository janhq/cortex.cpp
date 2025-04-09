include(FetchContent)

set(OPENSSL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/openssl)
set(OPENSSL_INCLUDE_DIR ${OPENSSL_INSTALL_DIR}/include)
if(EXISTS ${OPENSSL_INSTALL_DIR}/lib64)
  set(OPENSSL_ROOT_DIR ${OPENSSL_INSTALL_DIR}/lib64)
else()
  set(OPENSSL_ROOT_DIR ${OPENSSL_INSTALL_DIR}/lib)
endif()

find_package(OpenSSL REQUIRED)

FetchContent_Declare(curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG curl-8_13_0
)


FetchContent_MakeAvailableWithArgs(curl
    CURL_USE_LIBPSL=OFF
    BUILD_EXAMPLES=OFF
)

