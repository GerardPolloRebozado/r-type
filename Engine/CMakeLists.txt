cmake_minimum_required(VERSION 3.28)
project(Engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_library(EngineHeaders INTERFACE)
add_library(Engine::Headers ALIAS EngineHeaders)
target_include_directories(EngineHeaders INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_library(Engine STATIC ${SRC_FILES})
target_link_libraries(Engine PUBLIC EngineHeaders)

find_package(PkgConfig REQUIRED)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Engine PUBLIC raylib)
