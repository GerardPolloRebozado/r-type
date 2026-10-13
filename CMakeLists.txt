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
    set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cpm_cache" CACHE PATH "CPM source cache")
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

CPMAddPackage(
        NAME enet
        GITHUB_REPOSITORY zpl-c/enet
        GIT_TAG v2.7.0
)

set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

set(RLIMGUI_SOURCES
        ${rlImGui_SOURCE_DIR}/rlImGui.cpp
)
add_library(rlImGui STATIC ${RLIMGUI_SOURCES})
target_include_directories(rlImGui PUBLIC ${rlImGui_SOURCE_DIR})
target_link_libraries(rlImGui PUBLIC imgui raylib)

CPMAddPackage(
        NAME glaze
        GITHUB_REPOSITORY stephenberry/glaze
        GIT_TAG v9.0.0
)

option(EDITOR_BUILD "Build triggered by the Editor for a specific game" OFF)

add_subdirectory(Engine)
add_subdirectory(Serialization)
add_subdirectory(Plugins)

if (NOT EDITOR_BUILD)
    add_subdirectory(Editor)
    add_subdirectory(Tools)
    add_subdirectory(Games)
    add_subdirectory(Tests)
endif ()

if (EDITOR_BUILD)
    include(.editor_build.cmake OPTIONAL)
endif()
