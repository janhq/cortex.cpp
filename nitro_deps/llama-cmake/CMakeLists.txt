# For some reason llama does not provide really good namespace
# and current nitro code heavily relies on it's functionality
# e.g log.h, clip.h,... which is ambiguous
# we setup a better namespace here so nitro use the import path like
# llamalib/log.h,llamalib/clip.h ...

set(LLAMA_SOURCE_DIR "${CMAKE_SOURCE_DIR}/llama.cpp")



set(SRCS
  "${LLAMA_SOURCE_DIR}/llama.cpp"
  "${LLAMA_SOURCE_DIR}/ggml.c"
)
message("yooooo" ${SRCS})

add_library(llamalib INTERFACE)


# nitro only imports these headers
target_include_directories(llamalib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# have to make it PUBLIC
# meaning nitro header lists will include ambiguous paths like "common/log.h,common/common.h..."
# which smells
target_include_directories(llamalib INTERFACE ${LLAMA_SOURCE_DIR})


