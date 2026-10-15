#include "Scripting/LuaAPI/SceneAPI.hpp"
#include "Logging/Logger.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>
#include <string>

namespace Engine::LuaAPI {

void registerScene(sol::state_view lua) {
    sol::table scene = Internal::ensureNamespace(lua, "Scene");

    scene["load"] = [](const std::string& scenePath) {
        log_info("[Lua] Scene.load requested: " + scenePath + " (not implemented)");
    };
}

} // namespace Engine::LuaAPI
