#ifndef ENGINE_SCRIPTING_LUAAPI_PLUGINS_CONNECTIONSAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_PLUGINS_CONNECTIONSAPI_HPP_

namespace sol {
class state_view;
}
namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers Godot-style connection bindings in the `Component` namespace.
 *
 *   - `Component.connect(emitter, signal, target, method)` -> bool
 *     (idempotent: re-connecting the exact tuple returns true without
 *     duplicating, so script reloads that re-run onCreate stay clean;
 *     false on dead emitter/target or unknown signal)
 *   - `Component.disconnect(emitter, signal [, target [, method]])` -> bool
 *     (true when anything was removed)
 *   - `Component.getConnections(emitter)` -> array of
 *     {signal, target, method} tables
 *
 * Connections are plain data on @ref SignalConnections: the target method is
 * re-resolved at dispatch, so they survive save/load and script reloads.
 *
 * @param lua View of the shared Lua state (used at registration time only).
 * @param registry ECS registry holding the entities.
 */
void registerConnections(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_PLUGINS_CONNECTIONSAPI_HPP_
