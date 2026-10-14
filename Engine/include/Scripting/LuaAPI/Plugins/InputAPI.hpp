#ifndef ENGINE_SCRIPTING_LUAAPI_PLUGINS_INPUTAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_PLUGINS_INPUTAPI_HPP_

namespace sol {
class state_view;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers the `Input` namespace: keyboard polling, mouse position, and
 *        the `KEY.*` constant table.
 *        Mirrors Engine's InputPlugin / Controller (keyboard + mouse devices).
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 */
void registerInput(sol::state_view lua);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_PLUGINS_INPUTAPI_HPP_