# include(FetchContent)

# FetchContent_Declare(jsoncpp
#     GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
#     GIT_TAG 1.9.5
# )

# FetchContent_GetProperties(jsoncpp)
# if (NOT jsoncpp_POPULATED)
#     FetchContent_Populate(jsoncpp)
#     add_subdirectory(${jsoncpp_SOURCE_DIR} ${jsoncpp_BINARY_DIR})
#     message(${jsoncpp_SOURCE_DIR})
#     message(${jsoncpp_BINARY_DIR})
# endif ()

# #FetchContent_MakeAvailable(jsoncpp)

# FetchContent_MakeAvailable(jsoncpp)
# find_package(jsoncpp)

include(ExternalProject)

set(jsoncpp_install_prefix ${CMAKE_CURRENT_BINARY_DIR}/jsoncpp-lib)
ExternalProject_Add(jsoncpp-project
  GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
  GIT_TAG 1.9.6
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/jsoncpp-src
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/jsoncpp-build
  INSTALL_DIR ${jsoncpp_install_prefix}
  CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${jsoncpp_install_prefix}"
)

ExternalProject_Get_Property(jsoncpp-project INSTALL_DIR)
add_library(jsoncpp STATIC IMPORTED)
set(JSONCPP_LIBRARIES ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}jsoncpp${CMAKE_STATIC_LIBRARY_SUFFIX})
set(JSONCPP_INCLUDE_DIRS ${INSTALL_DIR}/include)

set_property(TARGET jsoncpp PROPERTY IMPORTED_LOCATION ${JSONCPP_LIBRARIES})
set_property(TARGET jsoncpp PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${JSONCPP_INCLUDE_DIRS})
add_dependencies(jsoncpp jsoncpp-project)




