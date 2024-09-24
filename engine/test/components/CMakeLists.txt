file(GLOB SRCS *.cc)
project(test-components)

enable_testing()

add_executable(${PROJECT_NAME} ${SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/modellist_utils.cc ${CMAKE_CURRENT_SOURCE_DIR}/../../config/yaml_config.cc ${CMAKE_CURRENT_SOURCE_DIR}/../../config/gguf_parser.cc)

find_package(Drogon CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon GTest::gtest GTest::gtest_main yaml-cpp::yaml-cpp
                                              ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(${PROJECT_NAME} PRIVATE httplib::httplib)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../)

add_test(NAME ${PROJECT_NAME}
         COMMAND ${PROJECT_NAME})
