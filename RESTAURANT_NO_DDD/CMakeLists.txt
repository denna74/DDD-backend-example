cmake_minimum_required(VERSION 3.20)
project(restaurant_no_ddd LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Bootstrap CPM.cmake
# ---------------------------------------------------------------------------
set(CPM_DOWNLOAD_VERSION 0.40.2)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT EXISTS "${CPM_DOWNLOAD_LOCATION}")
    message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}...")
    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        "${CPM_DOWNLOAD_LOCATION}"
        STATUS download_status
        TLS_VERIFY ON
    )
    list(GET download_status 0 download_code)
    if(NOT download_code EQUAL 0)
        message(FATAL_ERROR "Failed to download CPM.cmake: ${download_status}")
    endif()
endif()

include("${CPM_DOWNLOAD_LOCATION}")

# ---------------------------------------------------------------------------
# Dependencies via CPM
# ---------------------------------------------------------------------------
CPMAddPackage(
    NAME httplib
    GITHUB_REPOSITORY yhirose/cpp-httplib
    GIT_TAG v0.18.3
)

CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY nlohmann/json
    GIT_TAG v3.11.3
    OPTIONS "JSON_BuildTests OFF"
)

CPMAddPackage(
    NAME Catch2
    GITHUB_REPOSITORY catchorg/Catch2
    GIT_TAG v3.7.1
)

CPMAddPackage(
    NAME sqlite3
    URL https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip
    URL_HASH SHA256=5592243caf28b2cdef41e6ab58d25d653dfc53deded8450eb66072c929f030c4
    DOWNLOAD_ONLY YES
)

add_library(sqlite3_lib STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
target_include_directories(sqlite3_lib PUBLIC ${sqlite3_SOURCE_DIR})
add_library(SQLite::SQLite3 ALIAS sqlite3_lib)

# ---------------------------------------------------------------------------
# Library
# ---------------------------------------------------------------------------
file(GLOB_RECURSE LIB_SOURCES
    src/models/*.cpp
    src/repositories/*.cpp
    src/services/*.cpp
    src/controllers/*.cpp
    src/db.cpp
)
add_library(restaurant_no_ddd_lib ${LIB_SOURCES})
target_include_directories(restaurant_no_ddd_lib PUBLIC src)
target_link_libraries(restaurant_no_ddd_lib PUBLIC
    httplib::httplib
    nlohmann_json::nlohmann_json
    SQLite::SQLite3
)

# ---------------------------------------------------------------------------
# Main executable
# ---------------------------------------------------------------------------
add_executable(restaurant_no_ddd src/main.cpp)
target_link_libraries(restaurant_no_ddd PRIVATE restaurant_no_ddd_lib)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
add_executable(restaurant_no_ddd_tests ${TEST_SOURCES})
target_link_libraries(restaurant_no_ddd_tests PRIVATE
    restaurant_no_ddd_lib
    Catch2::Catch2WithMain
)

include(CTest)
list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/extras")
include(Catch)
catch_discover_tests(restaurant_no_ddd_tests)
