cmake_minimum_required(VERSION 3.28)
project(RTypeTests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(SerializationRoundTrip SerializationRoundTrip.cpp)
target_link_libraries(SerializationRoundTrip PRIVATE Engine Serialization)
target_compile_definitions(SerializationRoundTrip PRIVATE "RTYPE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
add_test(NAME SerializationRoundTrip COMMAND SerializationRoundTrip)

add_executable(PacketRoundTrip PacketRoundTrip.cpp)
target_link_libraries(PacketRoundTrip PRIVATE Engine)
add_test(NAME PacketRoundTrip COMMAND PacketRoundTrip)

add_executable(SceneManagerTest SceneManagerTest.cpp)
target_link_libraries(SceneManagerTest PRIVATE Engine)
add_test(NAME SceneManagerTest COMMAND SceneManagerTest)

add_executable(SceneFilesTest SceneFilesTest.cpp)
target_link_libraries(SceneFilesTest PRIVATE Engine Serialization)
add_test(NAME SceneFilesTest COMMAND SceneFilesTest)

add_executable(SceneSystemsTest SceneSystemsTest.cpp)
target_link_libraries(SceneSystemsTest PRIVATE Engine)
add_test(NAME SceneSystemsTest COMMAND SceneSystemsTest)

add_executable(ScaffoldRoundTrip ScaffoldRoundTrip.cpp ../Editor/src/ProjectSystemsBuild.cpp
        ../Editor/src/Scaffold.cpp ../Editor/src/Utils/Multiplatform.cpp)
target_include_directories(ScaffoldRoundTrip PRIVATE ${CMAKE_SOURCE_DIR}/Editor/include)
target_compile_definitions(ScaffoldRoundTrip PRIVATE "RTYPE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
target_link_libraries(ScaffoldRoundTrip PRIVATE Engine Serialization glaze::glaze)
add_test(NAME ScaffoldRoundTrip COMMAND ScaffoldRoundTrip)

add_executable(MultiplayerScenesTest MultiplayerScenesTest.cpp)
target_link_libraries(MultiplayerScenesTest PRIVATE Engine)
add_test(NAME MultiplayerScenesTest COMMAND MultiplayerScenesTest)

add_executable(WaveSpawnerNetworkTest WaveSpawnerNetworkTest.cpp)
target_link_libraries(WaveSpawnerNetworkTest PRIVATE Engine)
add_test(NAME WaveSpawnerNetworkTest COMMAND WaveSpawnerNetworkTest)

add_executable(AnimationAPITest AnimationAPITest.cpp)
target_link_libraries(AnimationAPITest PRIVATE Engine)
add_test(NAME AnimationAPITest COMMAND AnimationAPITest)
