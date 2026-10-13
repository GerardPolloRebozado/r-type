cmake_minimum_required(VERSION 3.28)
project(Engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_library(EngineHeaders INTERFACE)
add_library(Engine::Headers ALIAS EngineHeaders)
target_include_directories(EngineHeaders INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SRC_FILES
        src/Assets/AssetManager.cpp
        src/Assets/Sprite.cpp
        src/components/ShapeComponent.cpp
        src/Core/Application.cpp
        src/Core/Renderer.cpp
        src/Input/Controller.cpp
        src/Input/IController.cpp
        src/Input/InputManager.cpp
        src/Logging/LogBuffer.cpp
        src/Plugins/AudioPlugin.cpp
        src/Plugins/CameraPlugin.cpp
        src/Plugins/ColliderPlugin.cpp
        src/Plugins/HierarchyPlugin.cpp
        src/Plugins/InputPlugin.cpp
        src/Plugins/PluginExample.cpp
        src/Plugins/PositionPlugin.cpp
        src/Plugins/RotationPlugin.cpp
        src/Plugins/ShapePlugin.cpp
        src/Plugins/SpritePlugin.cpp
        src/Plugins/TextPlugin.cpp
        src/Systems/CameraSystem.cpp
        src/Systems/CollisionSystem.cpp
        src/Systems/HierarchySystem.cpp
        src/Systems/PositionSystem.cpp
        src/Systems/ShapeSystem.cpp
        src/Systems/SpriteSystem.cpp
        src/Systems/SystemExample.cpp
        src/Systems/TextSystem.cpp
        src/Resources/NetworkHostResource.cpp
        src/Systems/MultiplayerSystem.cpp
)

set(INC_FILES
        include/Assets/AssetManager.hpp
        include/Assets/Music.hpp
        include/Assets/Sound.hpp
        include/Assets/Sprites.hpp
        include/Components/AudioSource.hpp
        include/Components/CameraComponent.hpp
        include/Components/ColliderComponent.hpp
        include/Components/CollisionEvents.hpp
        include/Components/ComponentExample.hpp
        include/Components/Input.hpp
        include/Components/Name.hpp
        include/Components/Parent.hpp
        include/Components/PluginExampleEvents.hpp
        include/Components/Position.hpp
        include/Components/Rotation.hpp
        include/Components/ScreenSpace.hpp
        include/Components/ShapeComponent.hpp
        include/Components/SpriteComponent.hpp
        include/Components/Text.hpp
        include/Core/Application.hpp
        include/Core/Renderer.hpp
        include/ECS/Entity.hpp
        include/ECS/Registry.hpp
        include/ECS/System.hpp
        include/Errors/AError.hpp
        include/Errors/AssetErrors.hpp
        include/Errors/EcsErrors.hpp
        include/Errors/IError.hpp
        include/Inputs/Controller.hpp
        include/Inputs/IController.hpp
        include/Inputs/InputManager.hpp
        include/Logging/LogBuffer.hpp
        include/Logging/LogConfig.hpp
        include/Logging/Logger.hpp
        include/Plugins/AudioPlugin.hpp
        include/Plugins/CameraPlugin.hpp
        include/Plugins/ColliderPlugin.hpp
        include/Plugins/HierarchyPlugin.hpp
        include/Plugins/InputPlugin.hpp
        include/Plugins/IPlugin.hpp
        include/Resources/NetworkHostResource.hpp
        include/Plugins/PluginExample.hpp
        include/Plugins/PositionPlugin.hpp
        include/Plugins/RotationPlugin.hpp
        include/Plugins/ShapePlugin.hpp
        include/Plugins/SpritePlugin.hpp
        include/Plugins/TextPlugin.hpp
        include/Resources/StateResource.hpp
        include/Systems/AudioSystem.hpp
        include/Systems/CameraSystem.hpp
        include/Systems/CollisionSystem.hpp
        include/Systems/HierarchySystem.hpp
        include/Systems/PositionSystem.hpp
        include/Systems/ShapeSystem.hpp
        include/Systems/SpriteSystem.hpp
        include/Systems/StatesSystem.hpp
        include/Systems/SystemExample.hpp
        include/Systems/TextSystem.hpp
        include/Resources/PacketRouterResource.hpp
        include/Systems/MultiplayerSystem.hpp
        include/ENetContext.hpp
)

add_library(Engine STATIC ${SRC_FILES} ${INC_FILES})
target_link_libraries(Engine PUBLIC EngineHeaders)

set(ENGINE_LOG_LEVEL "ENGINE_LOG_INFO" CACHE STRING
        "Minimum Engine log level (ENGINE_LOG_ERROR, ENGINE_LOG_DEBUG, ENGINE_LOG_INFO)")
target_compile_definitions(Engine PUBLIC ENGINE_LOG_LEVEL=${ENGINE_LOG_LEVEL})

find_package(PkgConfig REQUIRED)
set(RAYLIB_CPP_INCLUDE_DIRS ${raylib-cpp_SOURCE_DIR}/include)
target_include_directories(Engine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${RAYLIB_CPP_INCLUDE_DIRS}
        ${RAYLIB_CPP_INCLUDE_DIRS}/raylib-cpp
)
target_link_libraries(Engine PUBLIC raylib)
target_link_libraries(Engine PUBLIC enet_static)
if (WIN32)
    target_link_libraries(MyEngine PRIVATE ws2_32 winmm)
endif ()
target_link_libraries(Engine PUBLIC bitsery)