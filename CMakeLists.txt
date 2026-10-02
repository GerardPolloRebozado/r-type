cmake_minimum_required(VERSION 3.28)
set(CMAKE_POLICY_DEFAULT_CMP0169 OLD)
project(G-ING-500-BAR-5-1-rtype-1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif ()
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS TRUE CACHE BOOL "Suppress developer warnings" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CPM_DOWNLOAD_VERSION 0.40.0)
if (NOT CPM_SOURCE_CACHE)
    set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_BINARY_DIR}/CPM_Cache" CACHE PATH "CPM source cache")
endif ()
file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/get_cpm.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cpm/get_cpm.cmake
)
include(${CMAKE_CURRENT_BINARY_DIR}/cpm/get_cpm.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

CPMAddPackage(
        NAME raylib
        GITHUB_REPOSITORY raysan5/raylib
        GIT_TAG master
        OPTIONS
        "BUILD_SHARED_LIBS OFF"
)

CPMAddPackage(
        NAME raylib-cpp
        GITHUB_REPOSITORY RobLoach/raylib-cpp
        GIT_TAG master
)

CPMAddPackage(
        NAME imgui
        GITHUB_REPOSITORY ocornut/imgui
        GIT_TAG docking
)
CPMAddPackage(
        NAME rlImGui
        GITHUB_REPOSITORY raylib-extras/rlImGui
        GIT_TAG main
)

add_subdirectory(Engine)
add_subdirectory(Editor)
add_subdirectory(Plugins)
add_subdirectory(Tools)

# games
add_subdirectory(Games)
