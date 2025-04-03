include(FetchContent)

set(CURL_USE_LIBPSL OFF)
set(BUILD_EXAMPLES OFF)
FetchContent_Declare(curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG curl-8_12_1
)

FetchContent_MakeAvailable(curl)
