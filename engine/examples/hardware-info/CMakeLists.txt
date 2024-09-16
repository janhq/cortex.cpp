cmake_minimum_required(VERSION 3.5)

project(hardware-info C CXX)

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

set(TARGET_NAME ${PROJECT_NAME})


if(NOT DEFINED HARDWARE_INFO_VERSION)
  set(HARDWARE_INFO_VERSION "default_version")
endif()
 
add_compile_definitions(HARDWARE_INFO_VERSION="${HARDWARE_INFO_VERSION}")

find_package(jsoncpp CONFIG REQUIRED)
find_package(Drogon CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(jinja2cpp CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(unofficial-minizip CONFIG REQUIRED)
find_package(LibArchive REQUIRED)
find_package(tabulate CONFIG REQUIRED)
find_package(CURL REQUIRED)

add_executable(
  ${TARGET_NAME} hardware_info.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/cpuid/cpu_info.cc
)

target_link_libraries(${TARGET_NAME} PRIVATE httplib::httplib)
target_link_libraries(${TARGET_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${TARGET_NAME} PRIVATE jinja2cpp)
target_link_libraries(${TARGET_NAME} PRIVATE CLI11::CLI11)
target_link_libraries(${TARGET_NAME} PRIVATE unofficial::minizip::minizip)
target_link_libraries(${TARGET_NAME} PRIVATE LibArchive::LibArchive)
target_link_libraries(${TARGET_NAME} PRIVATE tabulate::tabulate)
target_link_libraries(${TARGET_NAME} PRIVATE CURL::libcurl)
target_link_libraries(${TARGET_NAME} PRIVATE JsonCpp::JsonCpp Drogon::Drogon OpenSSL::SSL OpenSSL::Crypto yaml-cpp::yaml-cpp
  ${CMAKE_THREAD_LIBS_INIT})

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

aux_source_directory(controllers CTL_SRC)
aux_source_directory(services SERVICES_SRC)
aux_source_directory(common COMMON_SRC)
aux_source_directory(models MODEL_SRC)
aux_source_directory(cortex-common CORTEX_COMMON)
aux_source_directory(config CONFIG_SRC)
aux_source_directory(commands COMMANDS_SRC)

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../)

target_sources(${TARGET_NAME} PRIVATE ${COMMANDS_SRC} ${CONFIG_SRC} ${CTL_SRC} ${COMMON_SRC} ${SERVICES_SRC})
