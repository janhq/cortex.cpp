
set(LLAMA_SOURCE_DIR "${CMAKE_SOURCE_DIR}/llama.cpp")



# aux_source_directory(./../../llama.cpp SRCS)
set(SRCS
  "${LLAMA_SOURCE_DIR}/llama.cpp"
)

add_library(llamalib ${SRCS})


# target_include_directories(_rdkafka SYSTEM PUBLIC include)
target_include_directories(llamalib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include") # for "librdkafka/rdkafka.h"
target_include_directories(llamalib PUBLIC ${LLAMA_SOURCE_DIR})                 # Because weird logic with "include_next" is used.
# target_link_libraries(_rdkafka
#   PRIVATE
#     ch_contrib::lz4
#     ch_contrib::zlib
#     ch_contrib::zstd
#     OpenSSL::Crypto OpenSSL::SSL
# )
# if(WITH_SASL_CYRUS)
#     target_link_libraries(_rdkafka PRIVATE ch_contrib::sasl2)
# endif()

# file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/auxdir")

# configure_file(
#   "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
#   "${CMAKE_CURRENT_BINARY_DIR}/config.h"
#   IMMEDIATE @ONLY
# )

get_target_property(dirs llamalib INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()
message("end llama")