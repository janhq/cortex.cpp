cmake_minimum_required(VERSION 3.5)
project(nitro C CXX)

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
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build_deps/_install)
# This is the critical line for installing another package

# -----------------------------------Dynamically packaging python and libraries

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Find libintl
find_library(LIBINTL_LIBRARY NAMES intl)

# Find liblzma
find_library(LIBLZMA_LIBRARY NAMES lzma)

# Find Python 3 and its library
find_package(Python3 COMPONENTS Interpreter Development)

# Function to copy actual file from symlink
function(copy_dereferenced_symlink symlink_path destination)
  # Use 'readlink' to resolve the actual file path
  execute_process(
    COMMAND readlink -f ${symlink_path}
    OUTPUT_VARIABLE actual_file_path
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  # Copy the resolved actual file to the destination
  file(COPY ${actual_file_path} DESTINATION ${destination})
endfunction()

# Copy OpenSSL dylibs (actual files) to the destination folder
copy_dereferenced_symlink("${OPENSSL_INCLUDE_DIR}/../lib/libcrypto.dylib"
                          "${CMAKE_BINARY_DIR}/lib")
copy_dereferenced_symlink("${OPENSSL_INCLUDE_DIR}/../lib/libssl.dylib"
                          "${CMAKE_BINARY_DIR}/lib")

# Copy libintl dylib (actual file) to the destination folder, if found
if(LIBINTL_LIBRARY)
  copy_dereferenced_symlink("${LIBINTL_LIBRARY}" "${CMAKE_BINARY_DIR}/lib")
else()
  message(FATAL_ERROR "libintl not found. Please install libintl.")
endif()

# Copy liblzma dylib (actual file) to the destination folder, if found
if(LIBLZMA_LIBRARY)
  copy_dereferenced_symlink("${LIBLZMA_LIBRARY}" "${CMAKE_BINARY_DIR}/lib")
else()
  message(FATAL_ERROR "liblzma not found. Please install liblzma.")
endif()

# Check if Python and its library were found
if(Python3_FOUND AND Python3_Development_FOUND)
  # Locate libpython (the Python library)
  set(LIBPYTHON_PATH
      "${Python3_LIBRARY_DIRS}/libpython${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}.dylib"
  )

  # Copy libpython dylib to the destination folder
  copy_dereferenced_symlink("${LIBPYTHON_PATH}" "${CMAKE_BINARY_DIR}/lib")
else()
  message(FATAL_ERROR "Python 3 or its development libraries not found.")
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/start_nitro.sh"
               "${CMAKE_BINARY_DIR}/start_nitro.sh" COPYONLY)

# -----------------------End of packaging python

if(LLAMA_CUDA)
  cmake_minimum_required(VERSION 3.17)

  find_package(CUDAToolkit)
  if(CUDAToolkit_FOUND)
    message(STATUS "cuBLAS found")
    add_compile_definitions(GGML_USE_CUDA)
  endif()
endif()

if(DEBUG)
  message(STATUS "NITRO DEBUG IS ON")
  add_compile_definitions(ALLOW_ALL_CORS)
endif()

if(NOT DEFINED NITRO_VERSION)
  set(NITRO_VERSION "default_version")
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

add_compile_definitions(NITRO_VERSION="${NITRO_VERSION}")

add_subdirectory(llama.cpp/examples/llava)
add_subdirectory(llama.cpp)
add_subdirectory(whisper.cpp)

add_executable(${PROJECT_NAME} main.cc)

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon add_subdirectory(nitro_deps)
# target_link_libraries(${PROJECT_NAME} PRIVATE nitro_deps)
#
# and comment out the following lines
find_package(Drogon CONFIG REQUIRED)
target_link_libraries(
  ${PROJECT_NAME} PRIVATE Drogon::Drogon common llama whisper llava
                          ${CMAKE_THREAD_LIBS_INIT})

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
aux_source_directory(context CONTEXT_SRC)
aux_source_directory(models MODEL_SRC)
# aux_source_directory(filters FILTER_SRC) aux_source_directory(plugins
# PLUGIN_SRC)

# drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
# ${CMAKE_CURRENT_BINARY_DIR}) use the following line to create views with
# namespaces. drogon_create_views(${PROJECT_NAME}
# ${CMAKE_CURRENT_SOURCE_DIR}/views ${CMAKE_CURRENT_BINARY_DIR} TRUE)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_sources(${PROJECT_NAME} PRIVATE ${CTL_SRC} ${COMMON_SRC} ${CONTEXT_SRC})
# ${FILTER_SRC} ${PLUGIN_SRC} ${MODEL_SRC})
# ##############################################################################
# uncomment the following line for dynamically loading views set_property(TARGET
# ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)
