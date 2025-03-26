include(FetchContent)

# FetchContent_Declare(c-ares
#     GIT_REPOSITORY https://github.com/c-ares/c-ares.git
#     GIT_TAG v1.34.4
# )

# FetchContent_MakeAvailable(c-ares)
# set(C-ARES_INCLUDE_DIRS "${c-ares_BINARY_DIR}" "${c-ares_SOURCE_DIR}/include")

# set(C-ARES_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/lib)
# message("xxx="${C-ARES_LIBRARIES})

# FetchContent_Declare(jsoncpp
#     GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
#     GIT_TAG 1.9.5
# )

# FetchContent_MakeAvailable(jsoncpp)
# MESSAGE("jsoncpp_SOURCE_DIR" ${jsoncpp_SOURCE_DIR})

# set(JSONCPP_INCLUDE_DIRS "${jsoncpp_SOURCE_DIR}/include")
# set(JSONCPP_LIBRARIES "${jsoncpp_BINARY_DIR}")
set(USE_OSSP_UUID TRUE)


# FetchContent_Declare(openssl
#     GIT_REPOSITORY https://github.com/openssl/openssl.git
#     GIT_TAG openssl-3.4.1
# )

# FetchContent_MakeAvailable(openssl)

# set(OPENSSL_SSL_LIBRARY SSL)
# set(OPENSSL_CRYPTO_LIBRARY Crypto)
set(OPENSSL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/openssl)
set(OPENSSL_INCLUDE_DIR ${OPENSSL_INSTALL_DIR}/include)
set(OPENSSL_ROOT_DIR ${OPENSSL_INSTALL_DIR}/lib64)
# set(OPENSSL_INCLUDE_DIR "${openssl_BINARY_DIR}/include" "${openssl_BINARY_DIR}")


set(JSONCPP_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/jsoncpp-lib)
set(JSONCPP_LIBRARIES ${JSONCPP_INSTALL_DIR}/lib/libjsoncpp.so)
set(JSONCPP_INCLUDE_DIRS ${JSONCPP_INSTALL_DIR}/include)
message("JSONCPP_INCLUDE_DIRS="${JSONCPP_INCLUDE_DIRS})

set(C-ARES_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/c-ares-lib)
set(C-ARES_LIBRARIES ${C-ARES_INSTALL_DIR}/lib/libcares.so)
set(C-ARES_INCLUDE_DIRS ${C-ARES_INSTALL_DIR}/include)
# message("C-ARES_INCLUDE_DIRS="${C-ARES_INCLUDE_DIRS})
# FetchContent_GetProperties(openssl)
# if(NOT openssl_POPULATED)
#     FetchContent_Populate(openssl)
#     add_subdirectory(${openssl_SOURCE_DIR} ${openssl_BINARY_DIR})
# endif()
MESSAGE("sang1" ${OPENSSL_ROOT_DIR})
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
MESSAGE("sang1" ${OPENSSL_ROOT_DIR})
FetchContent_Declare(drogon
    GIT_REPOSITORY https://github.com/drogonframework/drogon.git
    GIT_TAG v1.9.10
)
MESSAGE("sang2")
FetchContent_MakeAvailable(drogon)
MESSAGE("sang3")

# add_library(drogon-1 INTERFACE)
# target_include_directories(drogon-1 INTERFACE ${drogon_SOURCE_DIR}/include ${drogon_BINARY_DIR}/include)
# target_link_libraries(drogon INTERFACE event event_pthreads)