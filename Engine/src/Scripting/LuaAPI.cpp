// Lua API orchestrator: opens standard libraries, then delegates to every
// registration module.
//
// Layout:
//   - `Scripting/LuaAPI/`           -> core, non-plugin modules (log, time,
//                                      entity, cross-component query, scene)
//   - `Scripting/LuaAPI/Plugins/`   -> one file per Engine plugin (Position,
//                                      Rotation, Sprite, Shape, Text,
//                                      Collider, Controller, Opacity, Parent,
//                                      Input, Connections)
#include "Scripting/LuaAPI.hpp"
#include "Core/LaunchOptions.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include "Scripting/LuaAPI/ComponentQueryAPI.hpp"
#include "Scripting/LuaAPI/EntityAPI.hpp"
#include "Scripting/LuaAPI/LogAPI.hpp"
#include "Scripting/LuaAPI/Plugins/AudioAPI.hpp"
#include "Scripting/LuaAPI/Plugins/ColliderAPI.hpp"
#include "Scripting/LuaAPI/Plugins/ConnectionsAPI.hpp"
#include "Scripting/LuaAPI/Plugins/ControllerAPI.hpp"
#include "Scripting/LuaAPI/Plugins/InputAPI.hpp"
#include "Scripting/LuaAPI/Plugins/MultiplayerAPI.hpp"
#include "Scripting/LuaAPI/Plugins/OpacityAPI.hpp"
#include "Scripting/LuaAPI/Plugins/ParentAPI.hpp"
#include "Scripting/LuaAPI/Plugins/PositionAPI.hpp"
#include "Scripting/LuaAPI/Plugins/RotationAPI.hpp"
#include "Scripting/LuaAPI/Plugins/ScriptAPI.hpp"
#include "Scripting/LuaAPI/Plugins/ShapeAPI.hpp"
#include "Scripting/LuaAPI/Plugins/SpriteAPI.hpp"
#include "Scripting/LuaAPI/Plugins/TextAPI.hpp"
#include "Scripting/LuaAPI/SceneAPI.hpp"
#include "Scripting/LuaAPI/TimeAPI.hpp"
#include <sol/sol.hpp>

namespace Engine::LuaAPI {

void openStandardLibraries(lua_State* L) {
    sol::state_view lua(L);
    lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::string, sol::lib::math,
                       sol::lib::debug, sol::lib::io, sol::lib::os, sol::lib::package);
}

void registerEngineAPI(lua_State* L, Registry& registry) {
    // One sol::state_view wraps the raw state. Each registration function takes
    // it by value, so lambdas capturing `lua` keep a valid copy instead of a
    // dangling reference to this short-lived local.
    sol::state_view lua(L);

    // --- Core (non-plugin) modules ---
    registerLog(lua);
    registerTime(lua);
    registerEntity(lua, registry);
    registerComponentQuery(lua, registry);
    registerScene(lua, registry);

    // --- One module per Engine plugin ---
    registerPosition(lua, registry);
    registerRotation(lua, registry);
    registerSprite(lua, registry);
    registerShape(lua, registry);
    registerText(lua, registry);
    registerCollider(lua, registry);
    registerConnections(lua, registry);
    registerController(lua, registry);
    registerOpacity(lua, registry);
    registerParent(lua, registry);
    registerInput(lua);
    registerMultiplayer(lua, registry);
    registerScript(lua, registry);
    registerAudio(lua, registry);

    // Command-line arguments, published as the Lua global `arg` (1-indexed) so
    // scripts can pick a run mode without any game logic in C++.
    if (registry.HasResource<LaunchOptions>()) {
        const auto& args = registry.GetResource<LaunchOptions>().args;
        sol::table argTable = lua.create_table();
        for (std::size_t i = 0; i < args.size(); ++i) {
            argTable[i + 1] = args[i];
        }
        lua["arg"] = argTable;
    }

    log_info("[LuaAPI] Engine API registered");
}

} // namespace Engine::LuaAPI