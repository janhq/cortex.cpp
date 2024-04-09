file(GLOB SRCS *.cc)
project(test-components)

enable_testing()

add_executable(${PROJECT_NAME} ${SRCS})

find_package(Drogon CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon GTest::gtest GTest::gmock 
                                              ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../)

add_test(NAME ${PROJECT_NAME}
         COMMAND ${PROJECT_NAME})
