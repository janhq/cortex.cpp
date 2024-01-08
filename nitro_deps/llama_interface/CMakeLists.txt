# For some reason llama does not provide really good namespace
# and current nitro code heavily relies on it's functionality
# e.g log.h, clip.h,... which is ambiguous
# we setup a better namespace here so nitro use the import path like
# llama_interface/log.h,llama_interface/clip.h ...


add_library(llama_interface INTERFACE)


# nitro only imports these headers
target_include_directories(llama_interface INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(llama_interface INTERFACE ${LLAMA_SOURCE_DIR})


