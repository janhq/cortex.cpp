cmake_minimum_required(VERSION 3.22)

project(cortex-server C CXX)

include(CheckIncludeFileCXX)

set(CMAKE_CXX_STANDARD 17)
MESSAGE("CMAKE_CXX_STANDARD="${CMAKE_CXX_STANDARD})

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# set(OPENSSL_USE_STATIC_LIBS TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add CORTEX_CQA option
# Enabling this option will currently break the build.
# Only for debugging
option(CORTEX_CQA "Enable Cortex Code Quality Assurance(DO NOT TURN THIS ON unless you know what you are doing)" OFF)

if(CORTEX_CQA)
    message(STATUS "CORTEX_CQA is ON: Enabling debug symbols, ASan, All Warnings, and treating Warnings as Errors")

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        add_compile_options(-fsanitize=address -Wall -Wextra -Wpedantic -Werror -g)
        add_link_options(-fsanitize=address)
    elseif(MSVC)
        # TODO: Sang, test on Windows and setup MSVC accordingly
        add_compile_options(/INFERASAN /Wall /WX /Zi)
        add_link_options(/DEBUG)
        message(WARNING "Address Sanitizer might require additional setup on MSVC. Please refer to MSVC documentation for ASan.")
    else()
        message(WARNING "Address Sanitizer and warning flags are not automatically configured for this compiler. Please configure them manually if supported.")
    endif()
else()
    message(STATUS "CORTEX_CQA is OFF.")
endif()

if(NOT CORTEX_CQA)
    message(STATUS "Setting up optimization flags for Release builds")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        # Add optimization flags for GCC/Clang
        add_compile_options($<$<CONFIG:Release>:-O3>)
        add_compile_options($<$<CONFIG:Release>:-flto>)
        add_link_options($<$<CONFIG:Release>:-flto>)
    elseif(MSVC)
        # Add optimization flags for MSVC
        # add_compile_options($<$<CONFIG:Release>:/O2>)
        # Optional: Link-time optimization
        # add_compile_options($<$<CONFIG:Release>:/GL>)
        # add_link_options($<$<CONFIG:Release>:/LTCG>)
    endif()
endif()

if(MSVC)
# set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  # add_compile_options(
  #     $<$<CONFIG:>:/MT> #---------|
  #     $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
  #     $<$<CONFIG:Release>:/MT> #--|
  #     $<$<CONFIG:Release>:/O2> #--|-- Optimize for speed in Release mode
  #     $<$<CONFIG:Release>:/Ob2> #-|-- Inline any suitable function
  # )

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

option(CMAKE_BUILD_TEST "Enable testing" ON)
option(CMAKE_BUILD_INJA_TEST "Enable inja example" OFF)
# if(CMAKE_BUILD_TEST)
  # add_subdirectory(test)
# endif()

# add_subdirectory(cli)

if(CMAKE_BUILD_INJA_TEST)
  add_subdirectory(examples/inja)
endif()
include(cmake/cli11.cmake)
include(cmake/libarchive.cmake)
include(cmake/curl.cmake)
include(cmake/drogon.cmake)
include(cmake/eventpp.cmake)
include(cmake/gtest.cmake)
include(cmake/indicators.cmake)
include(cmake/inja.cmake)

include(cmake/lfreist-hwinfo.cmake)
include(cmake/minizip.cmake)
include(cmake/sqlitecpp.cmake)
include(cmake/tabulate.cmake)
include(cmake/yaml-cpp.cmake)

find_library(JSONCPP
  NAMES jsoncpp
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/build_deps/jsoncpp-lib/lib/
)

## Generating openapi json
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../docs/static/openapi/cortex.json" JSON_CONTENT)
string(LENGTH "${JSON_CONTENT}" CONTENT_LENGTH)
message("Content length: ${CONTENT_LENGTH}")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cortex_openapi.h"
    "#pragma once\n"
    "#include <string>\n"
    "#include <sstream>\n\n"
    "namespace CortexOpenApi {\n"
)

set(CHUNK_SIZE 2000)
set(OFFSET 0)
set(CHUNK_INDEX 0)

while(${OFFSET} LESS ${CONTENT_LENGTH})
    math(EXPR REMAINING "${CONTENT_LENGTH} - ${OFFSET}")

    if(${REMAINING} LESS ${CHUNK_SIZE})
        string(SUBSTRING "${JSON_CONTENT}" ${OFFSET} ${REMAINING} CHUNK_CONTENT)
        math(EXPR OFFSET "${OFFSET} + ${REMAINING}")
    else()
        string(SUBSTRING "${JSON_CONTENT}" ${OFFSET} ${CHUNK_SIZE} CHUNK_CONTENT)
        math(EXPR OFFSET "${OFFSET} + ${CHUNK_SIZE}")
    endif()

    # Escape special characters
    string(REPLACE "\\" "\\\\" CHUNK_CONTENT "${CHUNK_CONTENT}")
    string(REPLACE "\"" "\\\"" CHUNK_CONTENT "${CHUNK_CONTENT}")
    string(REPLACE "\n" "\\n" CHUNK_CONTENT "${CHUNK_CONTENT}")

    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/cortex_openapi.h"
        "    inline std::string const json_part_${CHUNK_INDEX} = \"${CHUNK_CONTENT}\";\n"
    )

    math(EXPR CHUNK_INDEX "${CHUNK_INDEX} + 1")
endwhile()

# Add the get function that concatenates all parts
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/cortex_openapi.h"
    "\n    inline std::string GetOpenApiJson() {\n"
    "        std::ostringstream ss;\n"
)

# Add each part to the stream
set(INDEX 0)
while(${INDEX} LESS ${CHUNK_INDEX})
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/cortex_openapi.h"
        "        ss << json_part_${INDEX};\n"
    )
    math(EXPR INDEX "${INDEX} + 1")
endwhile()

file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/cortex_openapi.h"
    "        return ss.str();\n"
    "    }\n"
    "}\n"
)
## End of generating openapi json
MESSAGE("SOURCE_DIR"${CMAKE_CURRENT_SOURCE_DIR})
add_executable(${TARGET_NAME} main.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/cpuid/cpu_info.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/file_logger.cc

    ${CMAKE_CURRENT_SOURCE_DIR}/extensions/template_renderer.cc

    ${CMAKE_CURRENT_SOURCE_DIR}/utils/dylib_path_manager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/process/utils.cc

    ${CMAKE_CURRENT_SOURCE_DIR}/extensions/remote-engine/remote_engine.cc

)

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
# MESSAGE("ssl="${CMAKE_CURRENT_BINARY_DIR}/openssl-src/include)
target_include_directories(${TARGET_NAME} PRIVATE  ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/openssl/include)
target_include_directories(${TARGET_NAME} PRIVATE  ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/jsoncpp-lib/include)
target_include_directories(${TARGET_NAME} PRIVATE  ${CMAKE_CURRENT_BINARY_DIR}/../build/_deps/libarchive-src/libarchive)
target_include_directories(${TARGET_NAME} PRIVATE  ${CMAKE_CURRENT_BINARY_DIR}/../build/_deps/minizip-src)

target_link_libraries(${TARGET_NAME} PRIVATE minizip)
target_link_libraries(${TARGET_NAME} PRIVATE archive)
target_link_libraries(${TARGET_NAME} PRIVATE trantor)
target_link_libraries(${TARGET_NAME} PRIVATE drogon)
target_link_libraries(${TARGET_NAME} PRIVATE SQLiteCpp)
target_link_libraries(${TARGET_NAME} PRIVATE ${JSONCPP})
target_link_libraries(${TARGET_NAME} PRIVATE libcurl)
target_link_libraries(${TARGET_NAME} PRIVATE eventpp)
target_link_libraries(${TARGET_NAME} PRIVATE yaml-cpp)
target_link_libraries(${TARGET_NAME} PRIVATE hwinfo)
target_link_libraries(${TARGET_NAME} PRIVATE inja)

# ##############################################################################

if(CMAKE_CXX_STANDARD LESS 17)
  # With C++14, use boost to support any and std::string_view
  message(STATUS "use c++14")
  find_package(Boost 1.61.0 REQUIRED)
  target_include_directories(${TARGET_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
else()
  message(STATUS "use c++17")
endif()

aux_source_directory(controllers CTL_SRC)
aux_source_directory(repositories REPO_SRC)
aux_source_directory(services SERVICES_SRC)
aux_source_directory(models MODEL_SRC)
aux_source_directory(cortex-common CORTEX_COMMON)
aux_source_directory(config CONFIG_SRC)
aux_source_directory(database DB_SRC)
aux_source_directory(extensions EX_SRC)
aux_source_directory(migrations MIGR_SRC)
aux_source_directory(utils UTILS_SRC)

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )

target_sources(${TARGET_NAME} PRIVATE ${UTILS_SRC} ${CONFIG_SRC} ${CTL_SRC} ${COMMON_SRC} ${SERVICES_SRC} ${DB_SRC} ${EX_SRC} ${MIGR_SRC} ${REPO_SRC})

set_target_properties(${TARGET_NAME} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
