# WIP
- 3rd parties are installed using install_deps.sh, this script will fetch dependencies, including the object files/header files into
the path build_deps/_install
- for llama.cpp, we are using submodule instead, there is a proxy package llama_interface (the reason for this is to provide better header namespace,
because header files of llama are ambiguous (e.g common/log.h,common/common.h) and we import alot of llama codes)


# Apple 
- ZLib in apple command line tool is a .tbd that is reusable in other system (no need static) 
- OpenSSL has its own value of include

Static drogon usage
You need to add back these library when linking
```
target_link_libraries(${PROJECT_NAME} PRIVATE drogon brotlicommon brotlidec brotlienc cares resolv jsoncpp trantor OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB)
```
