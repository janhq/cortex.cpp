include(FetchContent)

FetchContent_Declare(sqlitecpp
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
    GIT_TAG 3.3.2
)

FetchContent_MakeAvailable(sqlitecpp)
