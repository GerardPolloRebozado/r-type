#ifndef ENGINE_SCRIPTING_LUAAPI_INTERNAL_HELPERS_HPP_
#define ENGINE_SCRIPTING_LUAAPI_INTERNAL_HELPERS_HPP_

#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include <sol/sol.hpp>

namespace Engine::LuaAPI::Internal {

/**
 * @brief Creates or fetches a global namespace table (e.g. "Component").
 *
 * Safe to call from many independent registration modules: the first caller
 * creates the table, subsequent callers just add their own keys. This removes
 * any ordering requirement between registration files.
 *
 * @param lua View of the shared Lua state (passed by value; cheap to copy).
 * @param name Global name, e.g. "Transform" or "Component".
 */
inline sol::table ensureNamespace(sol::state_view lua, const char* name) {
    sol::object existing = lua[name];
    if (existing.is<sol::table>()) {
        return existing.as<sol::table>();
    }
    sol::table t = lua.create_table();
    lua[name] = t;
    return t;
}

/**
 * @brief Adds a Position (and optionally Rotation) component if the entity lacks it.
 *
 * Most visual components are meaningless without a transform, so every binding
 * that adds a visual component calls this first. Keeps the ordering rule
 * implicit for script authors.
 */
inline void ensureTransform(Registry& registry, Entity entity, bool withRotation) {
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent<Position>(entity, Position{0.0f, 0.0f});
    }
    if (withRotation && !registry.HasComponent<Rotation>(entity)) {
        registry.AddComponent<Rotation>(entity, Rotation{0.0f});
    }
}

} // namespace Engine::LuaAPI::Internal

#endif // ENGINE_SCRIPTING_LUAAPI_INTERNAL_HELPERS_HPP_