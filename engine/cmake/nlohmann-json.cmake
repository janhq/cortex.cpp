# # include(FetchContent)

# # FetchContent_Declare(nlohmann-json
# #     GIT_REPOSITORY https://github.com/nlohmann/json.git
# #     GIT_TAG v3.11.3
# # )

# # FetchContent_MakeAvailable(nlohmann-json)
# # FetchContent_MakeAvailableWithArgs(nlohmann-json)

# include(ExternalProject)

# set(NLOHMANN_JSON_DIR ${CMAKE_CURRENT_BINARY_DIR}/nlohmann_json-lib)
# ExternalProject_Add(nlohmann_json-project
#   GIT_REPOSITORY https://github.com/nlohmann/json.git
#   GIT_TAG v3.11.3
#   SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/nlohmann_json-src
#   BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/nlohmann_json-build
#   INSTALL_DIR ${NLOHMANN_JSON_DIR}
#   CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${NLOHMANN_JSON_DIR}"
# )

# ExternalProject_Get_Property(nlohmann_json-project INSTALL_DIR)
# add_library(nlohmann_json STATIC IMPORTED)
# set(NLOHMANN_JSON_LIBRARIES ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}nlohmann_json${CMAKE_STATIC_LIBRARY_SUFFIX})
# set(NLOHMANN_JSON_INCLUDE_DIRS ${INSTALL_DIR}/include)
# set_property(TARGET nlohmann_json PROPERTY IMPORTED_LOCATION ${NLOHMANN_JSON_LIBRARIES})
# set_property(TARGET nlohmann_json PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${NLOHMANN_JSON_INCLUDE_DIRS})
# add_dependencies(nlohmann_json nlohmann_json-project)