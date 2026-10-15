#ifndef ENGINE_SCRIPTING_LUAAPI_COLLIDERAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_COLLIDERAPI_HPP_

namespace sol {
class state_view;
}
namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers `Component.addBoxCollider`.
 *        Mirrors Engine's ColliderPlugin / ColliderComponent.
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 * @param registry ECS registry used to add the Collider component.
 */
void registerCollider(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_COLLIDERAPI_HPP_