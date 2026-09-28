cmake_minimum_required(VERSION 3.28)
project(G-ING-500-BAR-5-1-rtype-1)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(Engine)
add_subdirectory(Games/R-Type)
add_subdirectory(Editor)
