#ifndef ENGINE_SCRIPTING_LUAAPI_ROTATIONAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_ROTATIONAPI_HPP_

namespace sol {
class state_view;
}
namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers the rotation half of the `Transform` namespace:
 *        getRotation / setRotation / rotate.
 *        Mirrors Engine's RotationPlugin.
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 * @param registry ECS registry used to read/write the Rotation component.
 */
void registerRotation(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_ROTATIONAPI_HPP_