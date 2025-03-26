include(FetchContent)

# FetchContent_Declare(openssl
#     GIT_REPOSITORY https://github.com/openssl/openssl.git
#     GIT_TAG openssl-3.4.1
# )

# FetchContent_MakeAvailable(openssl)

# set(OPENSSL_SSL_LIBRARY SSL)
# set(OPENSSL_CRYPTO_LIBRARY Crypto)
# set(OPENSSL_INCLUDE_DIR "${openssl_BINARY_DIR}/include" "${openssl_BINARY_DIR}")

# find_package(OpenSSL REQUIRED)
# SET(OPENSSL_FOUND TRUE)
set(OPENSSL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/openssl)
set(OPENSSL_INCLUDE_DIR ${OPENSSL_INSTALL_DIR}/include)
set(OPENSSL_ROOT_DIR ${OPENSSL_INSTALL_DIR}/lib64)

set(CURL_USE_LIBPSL OFF)
set(BUILD_EXAMPLES OFF)
FetchContent_Declare(curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG curl-8_12_1
)

FetchContent_MakeAvailable(curl)