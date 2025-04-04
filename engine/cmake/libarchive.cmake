include(FetchContent)

include(cmake/utils.cmake)
# set(ENABLE_TEST OFF)

MESSAGE("Start libarchive")
FetchContent_Declare(libarchive
    GIT_REPOSITORY https://github.com/libarchive/libarchive.git
    GIT_TAG v3.7.8
)

# FetchContent_MakeAvailable(libarchive)
MESSAGE("End libarchive")

FetchContent_MakeAvailableWithArgs(libarchive ENABLE_TEST=OFF)