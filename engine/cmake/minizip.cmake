include(FetchContent)

include(cmake/utils.cmake)
# set(ENABLE_TEST OFF)
# set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
MESSAGE("Start minizip")
FetchContent_Declare(minizip
    GIT_REPOSITORY https://github.com/zlib-ng/minizip-ng.git
    GIT_TAG 4.0.8
)

# FetchContent_MakeAvailable(minizip)
MESSAGE("End minizip")

FetchContent_MakeAvailableWithArgs(minizip MZ_BUILD_TESTS=OFF BUILD_SHARED_LIBS=OFF)