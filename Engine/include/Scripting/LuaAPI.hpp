#ifndef ENGINE_SCRIPTING_LUAAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_HPP_

struct lua_State;

namespace Engine {
class Registry;

/**
 * @namespace LuaAPI
 * @brief Registers the Engine's C++ API into the shared Lua state.
 *
 * The API is split into focused registration modules.
 *
 * `Scripting/LuaAPI/Plugins/` — one file per Engine plugin, each owning the Lua
 * functions for that plugin:
 *
 *   - `Position`  -> `Transform` (getPosition / setPosition / translate)
 *   - `Rotation`  -> `Transform` (getRotation / setRotation / rotate)
 *   - `Sprite`    -> `Component.addSprite` / `getSprite`
 *   - `Shape`     -> `Component.addShape`
 *   - `Text`      -> `Component.addText`
 *   - `Collider`  -> `Component.addBoxCollider`
 *   - `Controller`-> `Controller.isActionActive` / `getActiveActions` /
 *                     `addKeybind` / `hasController`, `Controller.KEYBOARD.*`
 *   - `Opacity`   -> `Component.setOpacity` / `getOpacity`
 *   - `Parent`    -> `Component.setParent` / `getParent` / `getChildren`
 *   - `Input`     -> `Input.isKey*`, `Input.getMousePosition`, `KEY.*`
 *   - `Multiplayer` -> `Net.register` / `on` / `off` / `send` / `broadcast` /
 *                       `onConnect` / `onDisconnect` / `isServer` / `localId`
 *
 * `Scripting/LuaAPI/` — core modules that are not plugins:
 *
 *   - `Log`             -> `log`, `print`
 *   - `Time`            -> `Time`
 *   - `Entity`          -> `Entity`
 *   - `ComponentQuery`  -> `Component.has` / `remove` (cross-component)
 *   - `Scene`           -> `Scene` (placeholder)
 *
 * Each module exposes a single `register*` function taking a `sol::state_view`
 * (by value) and, when registry access is needed, the `Registry`. Shared
 * utilities live in `LuaAPI/Internal/Helpers.hpp`.
 */
namespace LuaAPI {
/**
 * @brief Opens the standard Lua libraries on the state.
 * @param L Raw Lua state.
 */
void openStandardLibraries(lua_State* L);

/**
 * @brief Registers every Engine API module into the Lua state.
 *
 * Called once by @ref ScriptSystem on its first Update. After this returns,
 * Lua scripts can call `Entity.*`, `Transform.*`, `Component.*`, `Input.*`,
 * `Time.*`, `Scene.*`, `log.*` and `print`.
 *
 * @param L Raw Lua state.
 * @param registry ECS registry handed to the component-backed modules.
 */
void registerEngineAPI(lua_State* L, Registry& registry);
} // namespace LuaAPI
} // namespace Engine

#endif // ENGINE_SCRIPTING_LUAAPI_HPP_
