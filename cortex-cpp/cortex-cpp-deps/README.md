# WIP

# Apple 
- ZLib in apple command line tool is a .tbd that is reusable in other system (no need static) 
- OpenSSL has its own value of include

Static drogon usage
You need to add back these library when linking
```
target_link_libraries(${PROJECT_NAME} PRIVATE drogon brotlicommon brotlidec brotlienc cares resolv jsoncpp trantor OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB)
```
