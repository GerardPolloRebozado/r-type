#ifndef ENGINE_SCRIPTING_LUAAPI_INTERNAL_LUAVALUE_HPP_
#define ENGINE_SCRIPTING_LUAAPI_INTERNAL_LUAVALUE_HPP_

#include <glaze/glaze.hpp>
#include <set>
#include <sol/sol.hpp>

namespace Engine::LuaAPI::Internal {

/**
 * @brief Converts a Lua value into a JSON-serializable glaze value.
 *
 * Functions, threads and userdata become null. Tables are converted to arrays
 * when their keys are exactly 1..N, otherwise to objects with stringified keys.
 * @param seen Cycle guard shared across a single conversion.
 * @param depth Current nesting depth.
 */
glz::generic LuaToGeneric(const sol::object& obj, std::set<const void*>& seen, int depth = 0);

/**
 * @brief Converts a glaze value back into a Lua value.
 */
sol::object GenericToLua(sol::state_view lua, const glz::generic& value);

} // namespace Engine::LuaAPI::Internal

#endif // ENGINE_SCRIPTING_LUAAPI_INTERNAL_LUAVALUE_HPP_