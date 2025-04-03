include(FetchContent)

include(cmake/utils.cmake)

FetchContent_Declare(minizip
    GIT_REPOSITORY https://github.com/zlib-ng/minizip-ng.git
    GIT_TAG 4.0.8
)

if(MSVC)
    FetchContent_MakeAvailableWithArgs(minizip MZ_BUILD_TESTS=OFF BUILD_SHARED_LIBS=OFF)
else()
    FetchContent_MakeAvailable(minizip)
endif()
