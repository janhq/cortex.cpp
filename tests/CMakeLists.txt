
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../build_deps/_install)


add_subdirectory(integration)



# TODO: define function

add_executable(openai integration/openai.cpp)


find_package(GTest REQUIRED)
include(GoogleTest)

target_link_libraries(openai PRIVATE GTest::gtest_main)
gtest_discover_tests(openai)

