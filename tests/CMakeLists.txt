add_subdirectory(ut)


find_package(GTest REQUIRED)
include(GoogleTest)

# TODO: if we add more test case, create a function to build/link the test files
add_executable(ut_artifact ut/example.cpp)
target_link_libraries(ut_artifact PRIVATE nitrolib GTest::gtest_main)
gtest_discover_tests(ut_artifact)

