cmake_minimum_required(VERSION 3.14)
project(engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(SOURCES
        src/main.cpp)

#set(HEADERS)

add_executable(engine ${SOURCES} ${HEADERS})
target_include_directories(engine PRIVATE ${CMAKE_SOURCE_DIR}/include)

find_package(PkgConfig REQUIRED)
