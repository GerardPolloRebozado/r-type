cmake_minimum_required(VERSION 3.28)

file(GLOB_RECURSE OPENFRONT_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(Openfront ${OPENFRONT_SOURCES})

set_target_properties(Openfront PROPERTIES
    OUTPUT_NAME "openfront"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(Openfront PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Engine/include
)

target_link_libraries(Openfront PRIVATE 
    Engine 
    raylib
)

add_custom_command(TARGET Openfront POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Openfront> "${CMAKE_CURRENT_SOURCE_DIR}/openfront"
)

add_executable(OpenfrontTest
    tests/OpenfrontTest.cpp
    src/Map.cpp
    src/Player.cpp
    src/Combat.cpp
    src/Simulation.cpp
    src/NetworkServer.cpp
    src/NetworkClient.cpp
    src/NetworkManager.cpp
)

set_target_properties(OpenfrontTest PROPERTIES
    OUTPUT_NAME "OpenfrontTest"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(OpenfrontTest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Engine/include
)

target_link_libraries(OpenfrontTest PRIVATE 
    Engine 
    raylib
)

add_test(NAME OpenfrontTest COMMAND OpenfrontTest)
