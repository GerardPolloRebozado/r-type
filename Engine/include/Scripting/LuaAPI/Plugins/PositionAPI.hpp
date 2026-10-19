#ifndef ENGINE_SCRIPTING_LUAAPI_POSITIONAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_POSITIONAPI_HPP_

namespace sol {
class state_view;
}
namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers the position half of the `Transform` namespace:
 *        getPosition / setPosition / translate / getZIndex.
 *        Mirrors Engine's PositionPlugin / PositionSystem.
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 * @param registry ECS registry used to read/write the Position component.
 */
void registerPosition(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_POSITIONAPI_HPP_