#ifndef ENGINE_SCRIPTING_LUAAPI_PLUGINS_SCRIPTAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_PLUGINS_SCRIPTAPI_HPP_

namespace sol {
class state_view;
}

namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers script-attachment bindings in the `Component` namespace.
 *        Mirrors Engine's ScriptComponent / ScriptSystem.
 *
 *   - `Component.addScript(entity, scriptPath [, autoReload])` -> bool
 *   - `Component.getScriptPath(entity)` -> string or nil
 *
 * The path is relative to the project folder (same convention as the
 * `scriptPath` field of a scene), e.g. `"scripts/playerGun.lua"`. The script is
 * loaded by ScriptSystem on the next tick, so `onCreate`/`onUpdate` start
 * running after the current frame.
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 * @param registry ECS registry holding the entities.
 */
void registerScript(const sol::state_view& lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_PLUGINS_SCRIPTAPI_HPP_
