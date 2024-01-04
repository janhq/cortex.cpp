add_subdirectory(integration)


find_package(GTest REQUIRED)
include(GoogleTest)

# TODO: if we add more test case, create a function to build/link the test files
add_executable(openai integration/openai.cpp)
target_link_libraries(openai PRIVATE GTest::gtest_main)
gtest_discover_tests(openai)

