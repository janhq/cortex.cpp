# include(FetchContent)

# FetchContent_Declare(openssl
#     GIT_REPOSITORY https://github.com/openssl/openssl.git
#     GIT_TAG openssl-3.4.1
# )

# FetchContent_MakeAvailable(openssl)

# set(OPENSSL_SSL_LIBRARY SSL)
# set(OPENSSL_CRYPTO_LIBRARY Crypto)
# MESSAGE("openssl_SOURCE_DIR=" ${openssl_SOURCE_DIR})
# set(OPENSSL_INCLUDE_DIR "${openssl_SOURCE_DIR}/include" "${openssl_BINARY_DIR}")

include(ExternalProject)

set(OPENSSL_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl-src) # default path by CMake
set(OPENSSL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl)
set(OPENSSL_INCLUDE_DIR ${OPENSSL_INSTALL_DIR}/include)

set(OPENSSL_CONFIGURE_COMMAND ${OPENSSL_SOURCE_DIR}/config)
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

# set(OPENSSL_SSL_LIBRARY ${OPENSSL_INSTALL_DIR}/lib64/libssl.${OPENSSL_LIBRARY_SUFFIX})
# set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_INSTALL_DIR}/lib64/libcrypto.${OPENSSL_LIBRARY_SUFFIX})