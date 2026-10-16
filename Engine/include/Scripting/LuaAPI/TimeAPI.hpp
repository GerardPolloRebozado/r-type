#ifndef ENGINE_SCRIPTING_LUAAPI_TIMEAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_TIMEAPI_HPP_

namespace sol {
class state_view;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers the `Time` namespace (`Time.time`, `Time.deltaTime`).
 *
 * @param lua View of the shared Lua state.
 */
void registerTime(sol::state_view lua);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_TIMEAPI_HPP_