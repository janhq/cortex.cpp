file(GLOB SRCS *.cc)
project(test-components)

enable_testing()
 
add_executable(${PROJECT_NAME} 
  ${SRCS} 
  ${CMAKE_CURRENT_SOURCE_DIR}/../../config/yaml_config.cc 
  ${CMAKE_CURRENT_SOURCE_DIR}/../../config/gguf_parser.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../cli/commands/cortex_upd_cmd.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../cli/commands/server_stop_cmd.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../services/config_service.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../services/download_service.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../database/models.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/config_yaml_utils.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/file_manager_utils.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/curl_utils.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/system_info_utils.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../extensions/template_renderer.cc
)

# find_package(Drogon CONFIG REQUIRED)
# find_package(GTest CONFIG REQUIRED)
# find_package(yaml-cpp CONFIG REQUIRED)
# find_package(unofficial-minizip CONFIG REQUIRED)
# find_package(LibArchive REQUIRED)
# find_package(CURL REQUIRED)
# find_package(SQLiteCpp REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE GTest::gtest GTest::gtest_main GTest::gmock_main
                                              ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(${PROJECT_NAME} PRIVATE minizip)
target_link_libraries(${PROJECT_NAME} PRIVATE archive)
target_link_libraries(${PROJECT_NAME} PRIVATE libcurl)
target_link_libraries(${PROJECT_NAME} PRIVATE SQLiteCpp)

target_link_libraries(${PROJECT_NAME} PRIVATE eventpp)
target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE hwinfo)
target_link_libraries(${PROJECT_NAME} PRIVATE drogon) 

target_link_libraries(${PROJECT_NAME} PRIVATE inja)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../)

add_test(NAME ${PROJECT_NAME}
         COMMAND ${PROJECT_NAME})
