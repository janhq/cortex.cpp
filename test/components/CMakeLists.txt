file(GLOB SRCS *.cc)
project(test-components)

enable_testing()

add_executable(${PROJECT_NAME} ${SRCS})

find_package(Drogon CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

add_library(pyrunner STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../controllers/pyrunner.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../controllers/pyrunner.cc)

set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../build_deps/_install)

target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon GTest::gtest GTest::gmock pyrunner
                                              ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../
                                                   ${CMAKE_CURRENT_SOURCE_DIR}/../../build_deps/_install/include
                                                   ${CMAKE_CURRENT_SOURCE_DIR}/../../build_deps/_install/include/json)


add_test(NAME ${PROJECT_NAME}
         COMMAND ${PROJECT_NAME})
