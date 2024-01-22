# NOTE: file(GLOB) will not detect if we add new file to project
# in that case we have to run cmake manually again
function(find_object_files output_var)
    # Initialize an empty list to store the results
    set(result)

    # Loop through each directory provided as an argument
    foreach(dir IN LISTS ARGN)
        # Search for .cpp files in the current directory
        file(GLOB files "${dir}/*.cc")

        # Append found files to the result list
        list(APPEND result ${files})
    endforeach()

    # Set the output variable with the found files in the parent scope
    set(${output_var} ${result} PARENT_SCOPE)
endfunction()


find_package(Drogon CONFIG REQUIRED)
set(NITRO_SYSTEM_LIBS ${CMAKE_DL_LIBS})
set(NITRO_LINK_LIBS
  Drogon::Drogon 
  common 
  llava
  llama_interface
  ${NITRO_SYSTEM_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  )

# compile as library
# main/test will link to this library

find_object_files(ALL_OBJECT_FILES controllers utils)
add_library(nitrolib ${ALL_OBJECT_FILES})
target_link_libraries(nitrolib ${NITRO_LINK_LIBS})


# Nitro lib interface, TODO: a lot of implementations are defined in header files for some reason
target_include_directories(nitrolib PUBLIC include)
target_include_directories(nitrolib PUBLIC ../build_deps/_install/include)

# third party objects
target_link_directories(nitrolib PUBLIC ../build_deps/lib)

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon add_subdirectory(nitro_deps)
# target_link_libraries(${PROJECT_NAME} PRIVATE nitro_deps)
#
# and comment out the following lines


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

