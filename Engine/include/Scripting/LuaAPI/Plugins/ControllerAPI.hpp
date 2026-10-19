#ifndef ENGINE_SCRIPTING_LUAAPI_PLUGINS_CONTROLLERAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_PLUGINS_CONTROLLERAPI_HPP_

namespace sol {
class state_view;
}
namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers the `Controller` namespace: action-based input backed by
 *        the Engine's InputManager / Controller (one file per Engine plugin).
 *
 * Exposes:
 *   - `Controller.isActionActive(action [, playerId])`
 *   - `Controller.getActiveActions([playerId])`
 *   - `Controller.addKeybind(action, key [, deviceType [, composeKey [, playerId [, networked]]]])`
 *     `networked` (default false) marks the bind's action as sent to the server in `player_input`.
 *   - `Controller.hasController([playerId])`
 *   - `Controller.KEYBOARD / MOUSE / GAMEPAD / TOUCH` device constants
 *
 * Player IDs default to 1. The InputManager resource and the player controller
 * are created on demand so Lua-first workflows (no C++ recompilation, editor
 * preview with no C++ keybind setup) work. `Application::PreUpdate` already
 * calls `InputManager::Update()` every frame, so scripts only query / bind.
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 * @param registry ECS registry holding the InputManager resource.
 */
void registerController(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_PLUGINS_CONTROLLERAPI_HPP_
