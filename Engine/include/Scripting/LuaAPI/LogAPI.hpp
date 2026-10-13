#ifndef ENGINE_SCRIPTING_LUAAPI_LOGAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_LOGAPI_HPP_

namespace sol {
class state_view;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers the `log` namespace and the global `print`.
 * Corresponds to the Engine logging subsystem (Logging/Logger.hpp).
 *
 * @param lua View of the shared Lua state.
 */
void registerLog(sol::state_view lua);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_LOGAPI_HPP_