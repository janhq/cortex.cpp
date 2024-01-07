




set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../build_deps/_install
) # This is the critical line for installing another package

message(${CMAKE_PREFIX_PATH})
function(add_library_unity NAME MODE)
  set(SRCS ${ARGN})
  add_library(${NAME} OBJECT ${SRCS})
endfunction()


include_directories(include)

add_subdirectory(utils)
add_subdirectory(controllers)
set(NITRO_SYSTEM_LIBS ${CMAKE_DL_LIBS})
set(NITRO_LINK_LIBS
  Drogon::Drogon 
  common 
  llava
  llamalib
  ${NITRO_SYSTEM_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  )

# compile as lib
add_library(nitrolib SHARED ${ALL_OBJECT_FILES})
message(${ALL_OBJECT_FILES})
target_link_libraries(nitrolib ${NITRO_LINK_LIBS})

# third party import path
target_include_directories(nitrolib PUBLIC ../build_deps/_install/include)
# third party objects
target_link_directories(nitrolib PRIVATE ../build_deps/lib)

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

# aux_source_directory(filters FILTER_SRC) aux_source_directory(plugins
# PLUGIN_SRC) aux_source_directory(models MODEL_SRC)

# drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
# ${CMAKE_CURRENT_BINARY_DIR}) use the following line to create views with
# namespaces. drogon_create_views(${PROJECT_NAME}
# ${CMAKE_CURRENT_SOURCE_DIR}/views ${CMAKE_CURRENT_BINARY_DIR} TRUE)


# ${FILTER_SRC} ${PLUGIN_SRC} ${MODEL_SRC})
# ##############################################################################
# uncomment the following line for dynamically loading views set_property(TARGET
# ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)


get_target_property(dirs nitrolib INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()