cmake_minimum_required(VERSION 3.5)

project(cortex-cpp C CXX)
include(engines/cortex.llamacpp/engine.cmake)

# Build using CMAKE-JS
if(DEFINED CMAKE_JS_INC)
  if(WIN32)
  add_definitions(
    -DV8_COMPRESS_POINTERS
    -DV8_REVERSE_JSARGS
    -DV8_COMPRESS_POINTERS_IN_ISOLATE_CAGE
  )
  endif()
  include_directories(${CMAKE_JS_INC})
endif()

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
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build-deps/_install)
# This is the critical line for installing another package

if(LLAMA_CUDA)
  cmake_minimum_required(VERSION 3.17)

  find_package(CUDAToolkit)
  if(CUDAToolkit_FOUND)
    message(STATUS "cuBLAS found")
    add_compile_definitions(GGML_USE_CUDA)
  endif()
endif()

if(DEBUG)
  message(STATUS "CORTEX-CPP DEBUG IS ON")
  add_compile_definitions(ALLOW_ALL_CORS)
endif()

if(NOT DEFINED CORTEX_CPP_VERSION)
  set(CORTEX_CPP_VERSION "default_version")
endif()

if(APPLE)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm.*|ARM64)$")
    # MacOS silicon
    set(LLAMA_METAL_EMBED_LIBRARY ON)
    set(WHISPER_COREML 1)
  else()
    # MacOS amd64
    set(LLAMA_METAL OFF)
  endif()
endif()

if(DEFINED CMAKE_JS_INC)
  # define NPI_VERSION
  add_compile_definitions(NAPI_VERSION=8)
endif()

add_compile_definitions(CORTEX_CPP_VERSION="${CORTEX_CPP_VERSION}")

# add_subdirectory(test)

# Build using CMAKE-JS
if(DEFINED CMAKE_JS_INC)
  if(("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang") OR("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  endif()

  add_library(${PROJECT_NAME} SHARED addon.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/cpuid/cpu_info.cc
    ${CMAKE_JS_SRC}
  )

  if(WIN32)
    target_link_libraries(${PROJECT_NAME}
      PRIVATE
      msvcprt.lib
      msvcrt.lib
      vcruntime.lib
      ucrt.lib
      ${CMAKE_JS_LIB}
    )
  endif()
else() # Official build
  add_executable(${PROJECT_NAME} main.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/cpuid/cpu_info.cc
  )
endif()

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon add_subdirectory(cortex-cpp-deps)
# target_link_libraries(${PROJECT_NAME} PRIVATE cortex-cpp-deps)
#
# and comment out the following lines

find_package(Drogon CONFIG REQUIRED)


# Build using CMAKE-JS
if(DEFINED CMAKE_JS_INC)
  set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

  target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon
  ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_JS_LIB})

  if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
    # Generate node.lib
    execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
  endif()
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon
  ${CMAKE_THREAD_LIBS_INIT})
endif()
# ##############################################################################

if(CMAKE_CXX_STANDARD LESS 17)
  # With C++14, use boost to support any and std::string_view
  message(STATUS "use c++14")
  find_package(Boost 1.61.0 REQUIRED)
  target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
elseif(CMAKE_CXX_STANDARD LESS 20)
  message(STATUS "use c++17")
else()
  message(STATUS "use c++20")
endif()

aux_source_directory(controllers CTL_SRC)
aux_source_directory(common COMMON_SRC)
aux_source_directory(models MODEL_SRC)
aux_source_directory(cortex-common CORTEX_COMMON)
# aux_source_directory(filters FILTER_SRC) aux_source_directory(plugins
# PLUGIN_SRC) 

# drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
# ${CMAKE_CURRENT_BINARY_DIR}) use the following line to create views with
# namespaces. drogon_create_views(${PROJECT_NAME}
# ${CMAKE_CURRENT_SOURCE_DIR}/views ${CMAKE_CURRENT_BINARY_DIR} TRUE)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )
# ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_sources(${PROJECT_NAME} PRIVATE ${CTL_SRC} ${COMMON_SRC})
# ${FILTER_SRC} ${PLUGIN_SRC} ${MODEL_SRC})
# ##############################################################################
# uncomment the following line for dynamically loading views set_property(TARGET
# ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)
