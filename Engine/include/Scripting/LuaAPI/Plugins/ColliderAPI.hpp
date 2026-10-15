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
 * @brief Registers collider bindings in the `Component` namespace.
 *        Mirrors Engine's ColliderPlugin / ColliderComponent.
 *
 *   - `Component.addBoxCollider(entity, w, h [, isTrigger [, isStatic]])` -> bool
 *   - `Component.getColliderSize(entity)` -> {w, h, width, height} table or nil
 *   - `Component.setColliderSize(entity, w, h)` -> bool
 *   - `Component.getColliderEnabled(entity)` -> bool or nil
 *   - `Component.setColliderEnabled(entity, v)` -> bool
 *   - `Component.getColliderTrigger(entity)` -> bool or nil
 *   - `Component.setColliderTrigger(entity, v)` -> bool
 *   - `Component.getColliderStatic(entity)` -> bool or nil
 *   - `Component.setColliderStatic(entity, v)` -> bool
 *   - `Component.isColliding(entity)` -> bool (polls CollisionSystem contacts)
 *   - `Component.getCollisions(entity)` -> array table of other entity ids
 *
 * Removal uses the existing generic `Component.remove(entity, "Collider")`.
 *
 * @param lua View of the shared Lua state (used at registration time only).
 * @param registry ECS registry used to add the Collider component.
 */
void registerCollider(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_COLLIDERAPI_HPP_
