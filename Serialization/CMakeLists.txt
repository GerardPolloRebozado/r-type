cmake_minimum_required(VERSION 3.28)
project(Serialization)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
file(GLOB_RECURSE INC_FILES "include/*.hpp")

add_library(Serialization STATIC ${SRC_FILES} ${INC_FILES})

target_include_directories(Serialization PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Serialization PUBLIC
    Engine
    glaze::glaze
)
