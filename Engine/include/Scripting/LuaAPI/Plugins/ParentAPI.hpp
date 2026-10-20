#ifndef ENGINE_SCRIPTING_LUAAPI_PLUGINS_PARENTAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_PLUGINS_PARENTAPI_HPP_

namespace sol {
class state_view;
}
namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers parenting bindings in the `Component` namespace.
 *        Mirrors Engine's HierarchyPlugin / Parent component.
 *
 *   - `Component.setParent(child, parent [, relative])` -> bool
 *   - `Component.getParent(entity)` -> parent id or nil
 *   - `Component.getChildren(entity)` -> array table of child ids
 *   - `Component.flipX(entity)` -> bool
 *
 * `flipX` mirrors a whole rig: the entity's sprite flips and every descendant's
 * footprint is reflected about the entity's centre, so children (front sensors,
 * mounts) swap sides. It is a toggle, meant to be called once per direction
 * change; `Component.getSpriteFlip` reads the facing back.
 *
 * Detaching uses the existing generic `Component.remove(entity, "Parent")`.
 * `setParent` reuses `HierarchyPlugin::AttachChild`, so self-parenting and
 * cycles are rejected (returns false). The Parent component type is registered
 * idempotently here so Lua-first games work without HierarchyPlugin init.
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 * @param registry ECS registry holding the entities.
 */
void registerParent(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_PLUGINS_PARENTAPI_HPP_
