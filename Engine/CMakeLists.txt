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
        src/components/Animation.cpp
        src/components/ShapeComponent.cpp
        src/Core/Application.cpp
        src/Core/Renderer.cpp
        src/Core/VirtualResolution.cpp
        src/Input/Controller.cpp
        src/Input/IController.cpp
        src/Input/InputManager.cpp
        src/Logging/LogBuffer.cpp
        src/Logging/LogWriter.cpp
        src/Plugins/AudioPlugin.cpp
        src/Plugins/CameraPlugin.cpp
        src/Plugins/ColliderPlugin.cpp
        src/Plugins/HierarchyPlugin.cpp
        src/Plugins/InputPlugin.cpp
        src/Plugins/OpacityPlugin.cpp
        src/Plugins/PluginExample.cpp
        src/Plugins/SignalPlugin.cpp
        src/Plugins/PositionPlugin.cpp
        src/Plugins/RotationPlugin.cpp
        src/Plugins/ShapePlugin.cpp
        src/Plugins/SpritePlugin.cpp
        src/Plugins/AnimationPlugin.cpp
        src/Plugins/TextPlugin.cpp
        src/Plugins/WavePlugin.cpp
        src/Scripting/ScriptPlugin.cpp
        src/Scripting/ScriptSystem.cpp
        src/Scripting/ScriptVariableSync.cpp
        src/Scripting/SignalDispatch.cpp
        src/Scripting/LuaAPI.cpp
        src/Scripting/LuaAPI/LogAPI.cpp
        src/Scripting/LuaAPI/TimeAPI.cpp
        src/Scripting/LuaAPI/EntityAPI.cpp
        src/Scripting/LuaAPI/CameraAPI.cpp
        src/Scripting/LuaAPI/ComponentQueryAPI.cpp
        src/Scripting/LuaAPI/SceneAPI.cpp
        src/Scripting/LuaAPI/Internal/LuaValue.cpp
        src/Scripting/LuaAPI/Plugins/PositionAPI.cpp
        src/Scripting/LuaAPI/Plugins/RotationAPI.cpp
        src/Scripting/LuaAPI/Plugins/SpriteAPI.cpp
        src/Scripting/LuaAPI/Plugins/AnimationAPI.cpp
        src/Scripting/LuaAPI/Plugins/ShapeAPI.cpp
        src/Scripting/LuaAPI/Plugins/TextAPI.cpp
        src/Scripting/LuaAPI/Plugins/ColliderAPI.cpp
        src/Scripting/LuaAPI/Plugins/ConnectionsAPI.cpp
        src/Scripting/LuaAPI/Plugins/ControllerAPI.cpp
        src/Scripting/LuaAPI/Plugins/OpacityAPI.cpp
        src/Scripting/LuaAPI/Plugins/ParentAPI.cpp
        src/Scripting/LuaAPI/Plugins/ScriptAPI.cpp
        src/Scripting/LuaAPI/Plugins/InputAPI.cpp
        src/Scripting/LuaAPI/Plugins/AudioAPI.cpp
        src/Systems/AudioSystem.cpp
        src/Systems/CameraSystem.cpp
        src/Systems/CollisionSystem.cpp
        src/Systems/HierarchySystem.cpp
        src/Systems/PositionSystem.cpp
        src/Systems/ShapeSystem.cpp
        src/Systems/SpriteSystem.cpp
        src/Systems/AnimationSystem.cpp
        src/Systems/SystemExample.cpp
        src/Systems/TextSystem.cpp
        src/Systems/WaveSpawnerSystem.cpp
        src/Resources/NetworkHostResource.cpp
        src/Resources/SceneManager.cpp
        src/Systems/MultiplayerSystem.cpp
        src/Systems/ReplicationSystem.cpp
        src/Scripting/LuaAPI/Plugins/MultiplayerAPI.cpp
        include/Scripting/LuaAPI/Plugins/MultiplayerAPI.hpp
        src/Utils/EntityFlip.cpp
        src/Utils/WavePatterns.cpp
        src/Scripting/LuaAPI/WaveAPI.cpp
        src/Scripting/LuaAPI/PoolAPI.cpp
)

set(INC_FILES
        include/Assets/AssetManager.hpp
        include/Assets/Music.hpp
        include/Assets/ProjectPaths.hpp
        include/Assets/Sound.hpp
        include/Assets/Sprites.hpp
        include/Components/AudioSource.hpp
        include/Components/CameraComponent.hpp
        include/Components/ColliderComponent.hpp
        include/Components/CollisionEvents.hpp
        include/Components/CollisionLayers.hpp
        include/Components/ComponentExample.hpp
        include/Components/Input.hpp
        include/Components/Name.hpp
        include/Components/NetworkComponent.hpp
        include/Components/Opacity.hpp
        include/Components/Parent.hpp
        include/Components/PluginExampleEvents.hpp
        include/Components/Position.hpp
        include/Components/Rotation.hpp
        include/Components/ScreenSpace.hpp
        include/Components/ShapeComponent.hpp
        include/Components/SignalConnections.hpp
        include/Components/SpriteComponent.hpp
        include/Components/Text.hpp
        include/Core/Application.hpp
        include/Core/EditorMode.hpp
        include/Core/Renderer.hpp
        include/Core/VirtualResolution.hpp
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
        include/Logging/LogWriter.hpp
        include/Plugins/AudioPlugin.hpp
        include/Plugins/CameraPlugin.hpp
        include/Plugins/ColliderPlugin.hpp
        include/Plugins/HierarchyPlugin.hpp
        include/Plugins/InputPlugin.hpp
        include/Plugins/IPlugin.hpp
        include/Plugins/OpacityPlugin.hpp
        include/Plugins/SignalPlugin.hpp
        include/Resources/NetworkHostResource.hpp
        include/Resources/PlayerInputState.hpp
        include/Resources/PlayerSlotState.hpp
        include/Resources/PrefabSpawner.hpp
        include/Resources/ScriptVariableState.hpp
        include/Plugins/PluginExample.hpp
        include/Plugins/PositionPlugin.hpp
        include/Plugins/RotationPlugin.hpp
        include/Plugins/ShapePlugin.hpp
        include/Plugins/SpritePlugin.hpp
        include/Plugins/TextPlugin.hpp
        include/Plugins/WavePlugin.hpp
        include/Resources/StateResource.hpp
        include/Resources/WaveSchedule.hpp
        include/Reflection/ComponentReflect.hpp
        include/Scripting/ScriptComponent.hpp
        include/Scripting/ScriptPlugin.hpp
        include/Scripting/ScriptSystem.hpp
        include/Scripting/ScriptVariableSync.hpp
        include/Scripting/SignalDispatch.hpp
        include/Scripting/LuaAPI.hpp
        include/Scripting/LuaAPI/LogAPI.hpp
        include/Scripting/LuaAPI/TimeAPI.hpp
        include/Scripting/LuaAPI/EntityAPI.hpp
        include/Scripting/LuaAPI/CameraAPI.hpp
        include/Scripting/LuaAPI/ComponentQueryAPI.hpp
        include/Scripting/LuaAPI/SceneAPI.hpp
        include/Scripting/LuaAPI/Plugins/PositionAPI.hpp
        include/Scripting/LuaAPI/Plugins/RotationAPI.hpp
        include/Scripting/LuaAPI/Plugins/SpriteAPI.hpp
        include/Scripting/LuaAPI/Plugins/AnimationAPI.hpp
        include/Scripting/LuaAPI/Plugins/ShapeAPI.hpp
        include/Scripting/LuaAPI/Plugins/TextAPI.hpp
        include/Scripting/LuaAPI/Plugins/ColliderAPI.hpp
        include/Scripting/LuaAPI/Plugins/ConnectionsAPI.hpp
        include/Scripting/LuaAPI/Plugins/ControllerAPI.hpp
        include/Scripting/LuaAPI/Plugins/OpacityAPI.hpp
        include/Scripting/LuaAPI/Plugins/ParentAPI.hpp
        include/Scripting/LuaAPI/Plugins/ScriptAPI.hpp
        include/Scripting/LuaAPI/Plugins/InputAPI.hpp
        include/Scripting/LuaAPI/Plugins/AudioAPI.hpp
        include/Scripting/LuaAPI/Internal/Helpers.hpp
        include/Scripting/LuaAPI/Internal/LuaValue.hpp
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
        include/Systems/WaveSpawnerSystem.hpp
        include/Resources/PacketRouterResource.hpp
        include/Systems/MultiplayerSystem.hpp
        include/Systems/ReplicationSystem.hpp
        include/ENetContext.hpp
        include/Logging/LogBuffer.hpp
        include/Utils/EntityFlip.hpp
        src/Scripting/LuaAPI/Plugins/MultiplayerAPI.cpp
        include/Scripting/LuaAPI/Plugins/MultiplayerAPI.hpp
        include/Resources/PacketRegistryResource.hpp
        include/Plugins/MultiplayerPlugin.hpp
        include/Plugins/ReplicationPlugin.hpp
        src/Plugins/MultiplayerPlugin.cpp
        src/Plugins/ReplicationPlugin.cpp
        include/Utils/WavePatterns.hpp
        include/Scripting/LuaAPI/WaveAPI.hpp
)

add_library(Engine STATIC ${SRC_FILES} ${INC_FILES})
target_link_libraries(Engine PUBLIC EngineHeaders)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    find_package(Threads REQUIRED)
    target_link_libraries(Engine PUBLIC Threads::Threads)
endif()

set(ENGINE_LOG_LEVEL "ENGINE_LOG_INFO" CACHE STRING
        "Minimum Engine log level (ENGINE_LOG_ERROR, ENGINE_LOG_DEBUG, ENGINE_LOG_INFO)")
target_compile_definitions(Engine PUBLIC ENGINE_LOG_LEVEL=${ENGINE_LOG_LEVEL})

include(ProvideLua)
provide_lua()

set(RAYLIB_CPP_INCLUDE_DIRS ${raylib-cpp_SOURCE_DIR}/include)
target_include_directories(Engine PUBLIC
        ${LUA_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${RAYLIB_CPP_INCLUDE_DIRS}
        ${RAYLIB_CPP_INCLUDE_DIRS}/raylib-cpp
)
# sol2 is third-party: include as SYSTEM so its warnings don't trigger -Werror.
target_include_directories(Engine SYSTEM PUBLIC ${sol2_SOURCE_DIR}/include)
# sol2 3.3.1 + modern GCC emits -Wtemplate-body warnings; silence them if supported.
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-Wtemplate-body" HAS_TEMPLATE_BODY)
if (HAS_TEMPLATE_BODY)
    target_compile_options(Engine PUBLIC -Wno-template-body -Wno-error=template-body)
endif ()
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    CPMAddPackage(
            NAME lua
            GITHUB_REPOSITORY lua/lua
            GIT_TAG v5.4.7
            DOWNLOAD_ONLY YES
    )
    if(EXISTS "${lua_SOURCE_DIR}/lua.h")
        file(GLOB LUA_WASM_SOURCES "${lua_SOURCE_DIR}/*.c")
        list(REMOVE_ITEM LUA_WASM_SOURCES
                "${lua_SOURCE_DIR}/lua.c"
                "${lua_SOURCE_DIR}/luac.c"
                "${lua_SOURCE_DIR}/onelua.c")
        add_library(lua STATIC ${LUA_WASM_SOURCES})
        target_include_directories(lua PUBLIC "${lua_SOURCE_DIR}")
        target_link_libraries(Engine PUBLIC lua)
    endif()
else()
    target_link_libraries(Engine PUBLIC PkgConfig::LUA)
endif()
target_link_libraries(Engine PUBLIC raylib)
target_link_libraries(Engine PUBLIC enet_static)
if (WIN32)
    target_link_libraries(Engine PRIVATE ws2_32 winmm)
    # Narrowly scoped: Engine sources only (PRIVATE, not PUBLIC).
    # ENet pulls in <windows.h>, whose GDI/USER declarations collide with
    # raylib (Rectangle, CloseWindow, ShowCursor, DrawText, LoadImage...).
    # These must NOT be PUBLIC/global because (a) raylib's bundled GLFW
    # needs the full windows.h, and (b) dependents like the Editor use
    # portable-file-dialogs, which needs winuser.h (MessageBoxW, IDOK...).
    # Dependents are protected instead by Compat/EnetWrapper.hpp, which
    # defines these locally just before including <enet.h>.
    target_compile_definitions(Engine PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            NOGDI
            NOUSER
            MMNOSOUND
    )
endif ()
target_link_libraries(Engine PUBLIC glaze::glaze)