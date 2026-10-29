#ifndef ENGINE_SCRIPTING_LUAAPI_SCENEAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_SCENEAPI_HPP_

namespace sol {
class state_view;
}

namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers the `Scene` namespace: `Scene.load(name)` and `Scene.current()`.
 *
 * Both go through the registry's @ref SceneManager resource. `load` takes a
 * code scene name or a scene file path and switches at the end of the frame.
 * In multiplayer only the server's `load` counts: clients ignore their own
 * calls and follow the server.
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 * @param registry Registry holding the SceneManager resource.
 */
void registerScene(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_SCENEAPI_HPP_