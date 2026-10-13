project(LowLevelMultiplayerSDK)
set(CMAKE_BUILD_TYPE Debug)
add_executable(Client client.cpp
        PacketTypes.hpp)
target_include_directories(Client PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Client PRIVATE Engine)

add_executable(Server server.cpp
        PacketTypes.hpp)
target_include_directories(Server PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Server PRIVATE Engine)
