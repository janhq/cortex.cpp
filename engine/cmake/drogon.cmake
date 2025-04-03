include(FetchContent)

set(USE_OSSP_UUID TRUE)


set(OPENSSL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/openssl)
set(OPENSSL_INCLUDE_DIR ${OPENSSL_INSTALL_DIR}/include)
set(OPENSSL_ROOT_DIR ${OPENSSL_INSTALL_DIR}/lib64)


set(ZLIB_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/zlib/install)
set(ZLIB_INCLUDE_DIR ${ZLIB_INSTALL_DIR}/include)
if(MSVC)
    set(ZLIB_LIBRARY ${ZLIB_INSTALL_DIR}/lib/zlib.lib)
elseif(APPLE)
    set(ZLIB_LIBRARY ${ZLIB_INSTALL_DIR}/lib/libz.dylib)
else()
    set(ZLIB_LIBRARY ${ZLIB_INSTALL_DIR}/lib/libz.so)
endif()


set(JSONCPP_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/jsoncpp-lib)
if(MSVC)
    set(JSONCPP_LIBRARIES ${JSONCPP_INSTALL_DIR}/lib/jsoncpp.lib)
elseif(APPLE)
    set(JSONCPP_LIBRARIES ${JSONCPP_INSTALL_DIR}/lib/libjsoncpp.dylib)
else()
    set(JSONCPP_LIBRARIES ${JSONCPP_INSTALL_DIR}/lib/libjsoncpp.so)
endif()
set(JSONCPP_INCLUDE_DIRS ${JSONCPP_INSTALL_DIR}/include)

set(C-ARES_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/../build_deps/c-ares-lib)
if(MSVC)
    set(ARES_LIBRARIES ${ARES_INSTALL_DIR}/lib/cares.lib)
elseif(APPLE)
    set(ARES_LIBRARIES ${ARES_INSTALL_DIR}/lib/libcares.dylib)
else()
    set(ARES_LIBRARIES ${ARES_INSTALL_DIR}/lib/libcares.so)
endif()
set(C-ARES_INCLUDE_DIRS ${C-ARES_INSTALL_DIR}/include)

find_package(OpenSSL REQUIRED)

FetchContent_Declare(drogon
    GIT_REPOSITORY https://github.com/drogonframework/drogon.git
    GIT_TAG v1.9.10
)

FetchContent_MakeAvailableWithArgs(drogon
    BUILD_CTL=OFF
)
