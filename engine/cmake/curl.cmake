include(FetchContent)

FetchContent_Declare(curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG curl-8_12_1
)

FetchContent_MakeAvailableWithArgs(curl
    CURL_USE_LIBPSL=OFF
    BUILD_EXAMPLES=OFF
    OPENSSL_ROOT_DIR=${CMAKE_CURRENT_BINARY_DIR}/../build_deps/openssl/lib64
    OPENSSL_INCLUDE_DIR=${CMAKE_CURRENT_BINARY_DIR}/../build_deps/openssl/include/openssl
)
