# Set the minimum required version of cmake, project name and the language
cmake_minimum_required(VERSION 3.5)
project(janinfer_backend CXX)

option(BUILD_SINGLE_BINARY "Build only a single binary" OFF)

# Define the required packages
find_package(Drogon CONFIG REQUIRED)
find_package(PkgConfig)
find_package(valijson REQUIRED)
pkg_check_modules(OPENCV opencv4)

# Enforce C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Set binary output dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Specify the directories where the compiler can find included libraries
include_directories(/workspace/workdir/include /workspace/install/include /workspace/workdir/core/inference_backend/include ${OPENCV_INCLUDE_DIRS})
link_directories(/workspace/install/lib)

function(setup_target TARGET_SUFFIX CONTROLLER_PATHS)
    add_executable(${PROJECT_NAME}_${TARGET_SUFFIX} main.cc)

    foreach(PATH ${CONTROLLER_PATHS})
        file(GLOB_RECURSE CTL_SRC controllers/${PATH}/*.cc)
        target_sources(${PROJECT_NAME}_${TARGET_SUFFIX} PRIVATE ${CTL_SRC})
    endforeach()

    target_link_libraries(${PROJECT_NAME}_${TARGET_SUFFIX} PRIVATE
        Drogon::Drogon 
        grpc++
        grpc
        protobuf
        grpcclient
        aws-cpp-sdk-s3
        aws-cpp-sdk-core
        ${OPENCV_LIBRARIES}
        valijson
        sentencepiece)

    drogon_create_views(${PROJECT_NAME}_${TARGET_SUFFIX} ${CMAKE_CURRENT_SOURCE_DIR}/views ${CMAKE_CURRENT_BINARY_DIR})
    target_include_directories(${PROJECT_NAME}_${TARGET_SUFFIX} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/models)
endfunction()

# Iterate over directories in controllers
if(NOT BUILD_SINGLE_BINARY)
    # Iterate over directories in controllers to create multiple binaries
    file(GLOB CHILDREN RELATIVE ${CMAKE_SOURCE_DIR}/controllers ${CMAKE_SOURCE_DIR}/controllers/*)
    foreach(CHILD ${CHILDREN})
        if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/controllers/${CHILD})
            setup_target(${CHILD} "${CHILD}")
        endif()
    endforeach()
else()
    # Build only a single binary, include all controller directories
    file(GLOB CHILDREN RELATIVE ${CMAKE_SOURCE_DIR}/controllers ${CMAKE_SOURCE_DIR}/controllers/*)
    set(CONTROLLER_DIRS "")
    foreach(CHILD ${CHILDREN})
        if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/controllers/${CHILD})
            list(APPEND CONTROLLER_DIRS ${CHILD})
        endif()
    endforeach()
    setup_target(single_binary "${CONTROLLER_DIRS}")
endif()


# Add the tests subdirectory
add_subdirectory(test)
