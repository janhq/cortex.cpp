cmake_minimum_required(VERSION 3.25...3.31)

project(zlib_build LANGUAGES C)

include(ExternalProject)

ExternalProject_Add(
    zlib1
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    #--Download step--------------
    GIT_REPOSITORY https://github.com/ClausKlein/zlib.git
    GIT_TAG release/v1.2.5
    #--Update/Patch step----------
    #--Configure step-------------
    USES_TERMINAL_CONFIGURE TRUE
    CONFIGURE_COMMAND cmake -G "${CMAKE_GENERATOR}" -S ../zlib1 -B . -D CMAKE_BUILD_TYPE=$<CONFIG>
    #--Build step-----------------
    USES_TERMINAL_BUILD TRUE
    # BUILD_IN_SOURCE 1
    BUILD_COMMAND cmake --build . -j 8
    #--Install step---------------
    USES_TERMINAL_INSTALL TRUE
    INSTALL_COMMAND cmake --install <BINARY_DIR> --prefix=${CMAKE_INSTALL_PREFIX}
    #--Logging -------------------
    LOG_BUILD ON
)