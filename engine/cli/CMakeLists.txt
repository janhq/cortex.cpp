project(cortex C CXX)

include(CheckIncludeFileCXX)

check_include_file_cxx(any HAS_ANY)
check_include_file_cxx(string_view HAS_STRING_VIEW)
check_include_file_cxx(coroutine HAS_COROUTINE)
if(HAS_ANY
   AND HAS_STRING_VIEW
   AND HAS_COROUTINE)
  set(CMAKE_CXX_STANDARD 20)
elseif(HAS_ANY AND HAS_STRING_VIEW)
  set(CMAKE_CXX_STANDARD 17)
else()
  set(CMAKE_CXX_STANDARD 14)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(OPENSSL_USE_STATIC_LIBS TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
  add_compile_options(
      $<$<CONFIG:>:/MT> #---------|
      $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
      $<$<CONFIG:Release>:/MT> #--|
  )

  add_compile_options(/utf-8)
  add_definitions(-DUNICODE -D_UNICODE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DUNICODE /D_UNICODE")
endif()

if(NOT DEFINED CORTEX_VARIANT)
  set(CORTEX_VARIANT "prod")
endif()

set(TARGET_NAME ${PROJECT_NAME})
if(RENAME_EXE)
  if(CORTEX_VARIANT STREQUAL "beta")
    set(TARGET_NAME "${PROJECT_NAME}-beta")
  elseif(CORTEX_VARIANT STREQUAL "nightly")
    set(TARGET_NAME "${PROJECT_NAME}-nightly")
  else()
    set(TARGET_NAME ${PROJECT_NAME})
  endif()
endif()

if(NOT DEFINED CORTEX_CONFIG_FILE_PATH)
  set(CORTEX_CONFIG_FILE_PATH "user_home")
endif()

if(NOT DEFINED CORTEX_CPP_VERSION)
  set(CORTEX_CPP_VERSION "default_version")
endif()
 
add_compile_definitions(CORTEX_VARIANT="${CORTEX_VARIANT}")
add_compile_definitions(CORTEX_CPP_VERSION="${CORTEX_CPP_VERSION}")
add_compile_definitions(CORTEX_CONFIG_FILE_PATH="${CORTEX_CONFIG_FILE_PATH}")

find_package(jsoncpp CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(unofficial-minizip CONFIG REQUIRED)
find_package(LibArchive REQUIRED)
find_package(tabulate CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(Trantor CONFIG REQUIRED)
find_package(indicators CONFIG REQUIRED)
find_package(lfreist-hwinfo CONFIG REQUIRED)


add_executable(${TARGET_NAME} main.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/cpuid/cpu_info.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/normalize_engine.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/file_logger.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/dylib_path_manager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/command_line_parser.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../services/config_service.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../services/download_service.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../services/engine_service.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../services/model_service.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../services/inference_service.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../services/hardware_service.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../services/database_service.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/remote-engine/remote_engine.cc
    
    ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/python-engine/python_engine.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/template_renderer.cc

    ${CMAKE_CURRENT_SOURCE_DIR}/utils/easywsclient.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/download_progress.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/config_yaml_utils.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/file_manager_utils.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/curl_utils.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/system_info_utils.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/process/utils.cc
  )

target_link_libraries(${TARGET_NAME} PRIVATE CLI11::CLI11)
target_link_libraries(${TARGET_NAME} PRIVATE unofficial::minizip::minizip)
target_link_libraries(${TARGET_NAME} PRIVATE LibArchive::LibArchive)
target_link_libraries(${TARGET_NAME} PRIVATE tabulate::tabulate)
target_link_libraries(${TARGET_NAME} PRIVATE CURL::libcurl)
target_link_libraries(${TARGET_NAME} PRIVATE JsonCpp::JsonCpp OpenSSL::SSL OpenSSL::Crypto yaml-cpp::yaml-cpp
  ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${TARGET_NAME} PRIVATE SQLiteCpp)
target_link_libraries(${TARGET_NAME} PRIVATE Trantor::Trantor)
target_link_libraries(${TARGET_NAME} PRIVATE indicators::indicators)
target_link_libraries(${TARGET_NAME} PRIVATE lfreist-hwinfo::hwinfo)

# ##############################################################################

if(CMAKE_CXX_STANDARD LESS 17)
  # With C++14, use boost to support any and std::string_view
  message(STATUS "use c++14")
  find_package(Boost 1.61.0 REQUIRED)
  target_include_directories(${TARGET_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
elseif(CMAKE_CXX_STANDARD LESS 20)
  message(STATUS "use c++17")
else()
  message(STATUS "use c++20")
endif()

aux_source_directory(../common COMMON_SRC)
aux_source_directory(../models MODEL_SRC)
aux_source_directory(../cortex-common CORTEX_COMMON)
aux_source_directory(../config CONFIG_SRC)
aux_source_directory(commands COMMANDS_SRC)
aux_source_directory(../database DB_SRC)
aux_source_directory(../extensions EX_SRC)
 
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. )
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(${TARGET_NAME} PRIVATE ${COMMANDS_SRC} ${CONFIG_SRC} ${COMMON_SRC} ${DB_SRC} ${EX_SRC})

set_target_properties(${TARGET_NAME} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

if(UNIX AND NOT APPLE)
configure_file("${PROJECT_SOURCE_DIR}/../deps/linux/libvulkan.so" "${CMAKE_BINARY_DIR}/libvulkan.so" COPYONLY)
elseif(MSVC)
configure_file("${PROJECT_SOURCE_DIR}/../deps/windows/vulkan-1.dll" "${CMAKE_BINARY_DIR}/vulkan-1.dll" COPYONLY)
endif()