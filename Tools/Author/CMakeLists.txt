cmake_minimum_required(VERSION 3.28)
project(rtype-author)

add_library(Author STATIC
        src/Session.cpp
        src/Commands.cpp
        src/CommandsRead.cpp
        src/CommandsWrite.cpp
        src/McpStdio.cpp
)

target_include_directories(Author PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Author PUBLIC
        Engine
        Serialization
        UI
        glaze::glaze
)

target_compile_definitions(Author PRIVATE "RTYPE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

set_target_properties(Author PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
)

add_executable(rtype-author src/main.cpp)
target_link_libraries(rtype-author PRIVATE Author)
set_target_properties(rtype-author PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
)
