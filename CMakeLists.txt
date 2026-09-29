cmake_minimum_required(VERSION 3.28)

# Shared vcpkg_installed folder across all targets and builds
set(VCPKG_INSTALLED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed" CACHE PATH "Shared vcpkg installed directory")
list(APPEND CMAKE_PREFIX_PATH "${VCPKG_INSTALLED_DIR}/x64-linux")

# Check for vcpkg toolchain file if not provided by command line
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

project(G-ING-500-BAR-5-1-rtype-1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(Engine)
add_subdirectory(Editor)
add_subdirectory(Plugins)

# games
add_subdirectory(Games/R-Type)
add_subdirectory(Games/Example)
