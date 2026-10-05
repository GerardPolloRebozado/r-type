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
        src/Core/Application.cpp
        src/Systems/SystemExample.cpp
        src/Systems/TextSystem.cpp
        src/Systems/PositionSystem.cpp
        src/Systems/SpriteSystem.cpp
        src/Assets/Sprite.cpp
)

set(INC_FILES
        include/Assets/AssetManager.hpp
        include/Assets/Music.hpp
        include/Assets/Sound.hpp
        include/Components/AudioSource.hpp
        include/Components/ComponentExample.hpp
        include/Components/PluginExampleEvents.hpp
        include/Components/Position.hpp
        include/Core/Application.hpp
        include/ECS/Entity.hpp
        include/ECS/Registry.hpp
        include/ECS/System.hpp
        include/Plugins/AudioPlugin.hpp
        include/Plugins/IPlugin.hpp
        include/Plugins/PluginExample.hpp
        include/Plugins/TransformPlugin.hpp
        include/Resources/StateResource.hpp
        include/Systems/AudioSystem.hpp
        include/Systems/StatesSystem.hpp
        include/Systems/SystemExample.hpp
        include/Systems/TextSystem.hpp
        include/Assets/Sprites.hpp
)

add_library(Engine STATIC ${SRC_FILES})
target_link_libraries(Engine PUBLIC EngineHeaders)

find_package(PkgConfig REQUIRED)
set(RAYLIB_CPP_INCLUDE_DIRS ${raylib-cpp_SOURCE_DIR}/include)
target_include_directories(Engine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${RAYLIB_CPP_INCLUDE_DIRS}
        ${RAYLIB_CPP_INCLUDE_DIRS}/raylib-cpp
)
target_link_libraries(Engine PUBLIC raylib)
