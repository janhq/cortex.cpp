include(FetchContent)

FetchContent_Declare(yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.8.0
)

FetchContent_MakeAvailable(yaml-cpp)
