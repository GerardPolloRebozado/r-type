// Lua API orchestrator: opens standard libraries, then delegates to every
// registration module.
//
// Layout:
//   - `Scripting/LuaAPI/`           -> core, non-plugin modules (log, time,
//                                      entity, cross-component query, scene)
//   - `Scripting/LuaAPI/Plugins/`   -> one file per Engine plugin (Position,
//                                      Rotation, Sprite, Shape, Text,
//                                      Collider, Input)
#include "Scripting/LuaAPI.hpp"
#include "Logging/Logger.hpp"
#include "Scripting/LuaAPI/ComponentQueryAPI.hpp"
#include "Scripting/LuaAPI/EntityAPI.hpp"
#include "Scripting/LuaAPI/LogAPI.hpp"
#include "Scripting/LuaAPI/Plugins/ColliderAPI.hpp"
#include "Scripting/LuaAPI/Plugins/ControllerAPI.hpp"
#include "Scripting/LuaAPI/Plugins/InputAPI.hpp"
#include "Scripting/LuaAPI/Plugins/PositionAPI.hpp"
#include "Scripting/LuaAPI/Plugins/RotationAPI.hpp"
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
    registerScene(lua);

    // --- One module per Engine plugin ---
    registerPosition(lua, registry);
    registerRotation(lua, registry);
    registerSprite(lua, registry);
    registerShape(lua, registry);
    registerText(lua, registry);
    registerCollider(lua, registry);
    registerController(lua, registry);
    registerInput(lua);

    log_info("[LuaAPI] Engine API registered");
}

} // namespace Engine::LuaAPI