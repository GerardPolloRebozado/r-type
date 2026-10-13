cmake_minimum_required(VERSION 3.28)
project(RTypeTests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(SerializationRoundTrip SerializationRoundTrip.cpp)
target_link_libraries(SerializationRoundTrip PRIVATE Engine Serialization)

add_executable(PacketRoundTrip PacketRoundTrip.cpp)
target_link_libraries(PacketRoundTrip PRIVATE Engine)
