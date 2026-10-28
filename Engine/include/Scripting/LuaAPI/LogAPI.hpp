#ifndef ENGINE_SCRIPTING_LUAAPI_LOGAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_LOGAPI_HPP_

namespace sol {
class state_view;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers the `log` namespace and the global `print`.
 * `log.error` is a user error: the editor Console shows it and the terminal worker writes it.
 * `print`, `log.info`, and `log.debug` go to the terminal only.
 *
 * Corresponds to the Engine logging subsystem (Logging/Logger.hpp).
 *
 * @param lua View of the shared Lua state.
 */
void registerLog(sol::state_view lua);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_LOGAPI_HPP_