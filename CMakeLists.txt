cmake_minimum_required(VERSION 3.5)
project(nitro CXX)

include(CheckIncludeFileCXX)

option(BUILD_TESTS "Build the tests" ON)

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

# llama cpp server need llava example to work, this is for llama cpp server
set(LLAMA_BUILD_EXAMPLES ON)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(OPENSSL_USE_STATIC_LIBS TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
 # This is the critical line for installing another package

if(LLAMA_CUBLAS)
  cmake_minimum_required(VERSION 3.17)

  find_package(CUDAToolkit)
  if(CUDAToolkit_FOUND)
    message(STATUS "cuBLAS found")
    add_compile_definitions(GGML_USE_CUBLAS)
  endif()
endif()

if(DEBUG)
  message(STATUS "NITRO DEBUG IS ON")
  add_compile_definitions(ALLOW_ALL_CORS)
endif()

if(NOT DEFINED NITRO_VERSION)
  set(NITRO_VERSION "default_version")
endif()

add_compile_definitions(NITRO_VERSION="${NITRO_VERSION}")






set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build_deps/_install
) # This is the critical line for installing another package




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

# aux_source_directory(controllers CTL_SRC)
# aux_source_directory(filters FILTER_SRC) aux_source_directory(plugins
# PLUGIN_SRC) aux_source_directory(models MODEL_SRC)

# drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
# ${CMAKE_CURRENT_BINARY_DIR}) use the following line to create views with
# namespaces. drogon_create_views(${PROJECT_NAME}
# ${CMAKE_CURRENT_SOURCE_DIR}/views ${CMAKE_CURRENT_BINARY_DIR} TRUE)

# target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# ${CMAKE_CURRENT_SOURCE_DIR}/models)
# target_sources(${PROJECT_NAME} PRIVATE ${CTL_SRC})

# ${FILTER_SRC} ${PLUGIN_SRC} ${MODEL_SRC})
# ##############################################################################
# uncomment the following line for dynamically loading views set_property(TARGET
# ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)

add_executable(${PROJECT_NAME} main.cc)

# interface package for llama (better namespace/dependency declaration) 
add_subdirectory(nitro_deps/llama_interface)
add_subdirectory(llama.cpp)


# init nitrolib
# all 3rd party dependencies are declared nitrolib instead
add_subdirectory(src)
target_link_libraries(${PROJECT_NAME} PRIVATE nitrolib)


if (BUILD_TESTS)
  message("Building test")
  enable_testing()
  add_subdirectory(tests)
endif()