#ifndef ENGINE_SCRIPTING_LUAAPI_SCENEAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_SCENEAPI_HPP_

namespace sol {
class state_view;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers the `Scene` namespace (currently a placeholder for future
 * runtime scene loading).
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 */
void registerScene(sol::state_view lua);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_SCENEAPI_HPP_