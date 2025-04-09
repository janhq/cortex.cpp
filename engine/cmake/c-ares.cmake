include(ExternalProject)

# c-ares setup
set(CARES_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/c-ares-lib)
ExternalProject_Add(c-ares-project
  GIT_REPOSITORY https://github.com/c-ares/c-ares.git
  GIT_TAG v1.34.4
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/c-ares-src
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/c-ares-build
  INSTALL_DIR ${CARES_INSTALL_DIR}
  CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CARES_INSTALL_DIR}"
)

ExternalProject_Get_Property(c-ares-project INSTALL_DIR)
add_library(c-ares STATIC IMPORTED)
set(CARES_LIBRARIES ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}cares${CMAKE_STATIC_LIBRARY_SUFFIX})
set(CARES_INCLUDE_DIRS ${INSTALL_DIR}/include)
set_property(TARGET c-ares PROPERTY IMPORTED_LOCATION ${CARES_LIBRARIES})
set_property(TARGET c-ares PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CARES_INCLUDE_DIRS})
add_dependencies(c-ares c-ares-project)