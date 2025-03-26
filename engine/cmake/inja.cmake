include(FetchContent)
MESSAGE("Start inja")
FetchContent_Declare(inja
    GIT_REPOSITORY https://github.com/pantor/inja.git
    GIT_TAG v3.4.0
)

FetchContent_MakeAvailable(inja)
MESSAGE("Stop inja")
