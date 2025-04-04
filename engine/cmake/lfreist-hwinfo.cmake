include(FetchContent)

FetchContent_Declare(lfreist-hwinfo
    GIT_REPOSITORY https://github.com/lfreist/hwinfo.git
    GIT_TAG main
)

FetchContent_MakeAvailable(lfreist-hwinfo)
