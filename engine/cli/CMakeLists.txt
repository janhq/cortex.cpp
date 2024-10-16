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
endif()

if(DEBUG)
  message(STATUS "CORTEX-CPP DEBUG IS ON")
  add_compile_definitions(ALLOW_ALL_CORS)
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
find_package(httplib CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(unofficial-minizip CONFIG REQUIRED)
find_package(LibArchive REQUIRED)
find_package(tabulate CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(eventpp CONFIG REQUIRED)
find_package(Trantor CONFIG REQUIRED)

add_executable(${TARGET_NAME} main.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/cpuid/cpu_info.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/file_logger.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../controllers/command_line_parser.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../services/download_service.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../services/engine_service.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../services/model_service.cc
  )

target_link_libraries(${TARGET_NAME} PRIVATE httplib::httplib)
target_link_libraries(${TARGET_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${TARGET_NAME} PRIVATE CLI11::CLI11)
target_link_libraries(${TARGET_NAME} PRIVATE unofficial::minizip::minizip)
target_link_libraries(${TARGET_NAME} PRIVATE LibArchive::LibArchive)
target_link_libraries(${TARGET_NAME} PRIVATE tabulate::tabulate)
target_link_libraries(${TARGET_NAME} PRIVATE CURL::libcurl)
target_link_libraries(${TARGET_NAME} PRIVATE JsonCpp::JsonCpp OpenSSL::SSL OpenSSL::Crypto yaml-cpp::yaml-cpp
  ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${TARGET_NAME} PRIVATE SQLiteCpp)
target_link_libraries(${TARGET_NAME} PRIVATE eventpp::eventpp)
target_link_libraries(${TARGET_NAME} PRIVATE Trantor::Trantor)

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

# aux_source_directory(../controllers CTL_SRC)
# aux_source_directory(../services SERVICES_SRC)
aux_source_directory(../common COMMON_SRC)
aux_source_directory(../models MODEL_SRC)
aux_source_directory(../cortex-common CORTEX_COMMON)
aux_source_directory(../config CONFIG_SRC)
aux_source_directory(../commands COMMANDS_SRC)
aux_source_directory(../database DB_SRC)
 
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. )

target_sources(${TARGET_NAME} PRIVATE ${COMMANDS_SRC} ${CONFIG_SRC} ${COMMON_SRC} ${DB_SRC})
