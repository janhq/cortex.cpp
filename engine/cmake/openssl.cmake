include(ExternalProject)

# Define directories for dependencies
set(DEPS_DIR ${CMAKE_BINARY_DIR}/)
set(ZLIB_INSTALL_DIR ${DEPS_DIR}/zlib/install)

ExternalProject_Add(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG v1.2.11
  CMAKE_ARGS
  -DBUILD_SHARED_LIBS=ON
  -DCMAKE_INSTALL_PREFIX=${ZLIB_INSTALL_DIR}
)

# Set variables for zlib include and library directories
set(ZLIB_INCLUDE_DIR ${ZLIB_INSTALL_DIR}/include)
set(ZLIB_LIBRARY_DIR ${ZLIB_INSTALL_DIR}/lib)

set(OPENSSL_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl-src) # default path by CMake
set(OPENSSL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl)
set(OPENSSL_INCLUDE_DIR ${OPENSSL_INSTALL_DIR}/include)

set(OPENSSL_CONFIGURE_COMMAND ${OPENSSL_SOURCE_DIR}/config)
if(MSVC)
  Externoject_Add(
    SOURCE_DIR ${OPENSSL_SOURCE_DIR}
    GIT_REPOSITORY https://github.com/openssl/openssl.git
    GIT_TAG openssl-3.4.1
    USES_TERMINAL_DOWNLOAD TRUE
    CONFIGURE_COMMAND perl Configure VC-WIN64A no-idea no-mdc2 no-rc5 --prefix=<INSTALL_DIR> --openssldir=<INSTALL_DIR>/ssl
    BUILD_IN_SOURCE 1
    BUILD_COMMAND nmake
    INSTALL_COMMAND nmake install
    INSTALL_DIR ${OPENSSL_INSTALL_DIR}
  )
else()
  ExternalProject_Add(
    OpenSSL
    SOURCE_DIR ${OPENSSL_SOURCE_DIR}
    GIT_REPOSITORY https://github.com/openssl/openssl.git
    GIT_TAG openssl-3.4.1
    USES_TERMINAL_DOWNLOAD TRUE
    CONFIGURE_COMMAND
    ${OPENSSL_CONFIGURE_COMMAND}
    --prefix=${OPENSSL_INSTALL_DIR}
    --openssldir=${OPENSSL_INSTALL_DIR}
    BUILD_COMMAND make
    TEST_COMMAND ""
    INSTALL_COMMAND make install
    INSTALL_DIR ${OPENSSL_INSTALL_DIR}
  )
endif()



# We cannot use find_library because ExternalProject_Add() is performed at build time.
# And to please the property INTERFACE_INCLUDE_DIRECTORIES,
# we make the include directory in advance.
file(MAKE_DIRECTORY ${OPENSSL_INCLUDE_DIR})

add_library(OpenSSL::SSL STATIC IMPORTED GLOBAL)
set_property(TARGET OpenSSL::SSL PROPERTY IMPORTED_LOCATION ${OPENSSL_INSTALL_DIR}/lib64/libssl.${OPENSSL_LIBRARY_SUFFIX})
set_property(TARGET OpenSSL::SSL PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INCLUDE_DIR})
add_dependencies(OpenSSL::SSL OpenSSL)

add_library(OpenSSL::Crypto STATIC IMPORTED GLOBAL)
set_property(TARGET OpenSSL::Crypto PROPERTY IMPORTED_LOCATION ${OPENSSL_INSTALL_DIR}/lib64/libcrypto.${OPENSSL_LIBRARY_SUFFIX})
set_property(TARGET OpenSSL::Crypto PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INCLUDE_DIR})
add_dependencies(OpenSSL::Crypto OpenSSL)