#include "Scripting/LuaAPI/Plugins/ColliderAPI.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/CollisionEvents.hpp"
#include "Components/ShapeComponent.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include "Systems/CollisionSystem.hpp"
#include <optional>
#include <sol/sol.hpp>

namespace Engine::LuaAPI {

namespace {

// Shared add helper: returns false on dead entities so Lua can branch.
// Free function (not a local lambda) so overload forwarders can capture only
// `registry`, which outlives every Lua closure. Capturing a local lambda by
// reference here would "dangle" once this function returns.
bool addBoxColliderImpl(Registry& registry, Entity entity, float width, float height,
                        bool isTrigger, bool isStatic) {
    if (!registry.IsEntityAlive(entity)) {
        return false;
    }
    Internal::ensureTransform(registry, entity, false);
    ColliderComponent cc;
    cc.enabled = true;
    cc.isTrigger = isTrigger;
    cc.isStatic = isStatic;
    ShapeComponent shape;
    shape.type = ShapeType::RECTANGLE;
    shape.width = width;
    shape.height = height;
    shape.filled = true;
    cc.shape = shape;
    registry.AddComponent<ColliderComponent>(entity, cc);
    if (!registry.HasComponent<CollisionEvents>(entity)) {
        registry.AddComponent<CollisionEvents>(entity, CollisionEvents{});
    }
    return true;
}

} // namespace

void registerCollider(sol::state_view lua, Registry& registry) {
    sol::table component = Internal::ensureNamespace(lua, "Component");

    component["addBoxCollider"] = sol::overload(
        [&registry](Entity entity, float width, float height) {
            return addBoxColliderImpl(registry, entity, width, height, false, false);
        },
        [&registry](Entity entity, float width, float height, bool isTrigger) {
            return addBoxColliderImpl(registry, entity, width, height, isTrigger, false);
        },
        [&registry](Entity entity, float width, float height, bool isTrigger, bool isStatic) {
            return addBoxColliderImpl(registry, entity, width, height, isTrigger, isStatic);
        });

    component["getColliderSize"] = [&registry](sol::this_state ts, Entity entity) -> sol::object {
        sol::state_view lua(ts);
        if (!registry.HasComponent<ColliderComponent>(entity)) {
            return sol::make_object(lua, sol::lua_nil);
        }
        const auto& cc = registry.GetComponent<ColliderComponent>(entity);
        if (!cc.shape.has_value()) {
            return sol::make_object(lua, sol::lua_nil);
        }
        sol::table t = lua.create_table();
        t["w"] = cc.shape->width;
        t["h"] = cc.shape->height;
        // width/height aliases, matching getSpriteScale's x/y style.
        t["width"] = cc.shape->width;
        t["height"] = cc.shape->height;
        return sol::make_object(lua, t);
    };

    component["setColliderSize"] = [&registry](Entity entity, float width, float height) -> bool {
        if (!registry.HasComponent<ColliderComponent>(entity)) {
            return false;
        }
        auto& cc = registry.GetComponent<ColliderComponent>(entity);
        if (!cc.shape.has_value()) {
            ShapeComponent shape;
            shape.type = ShapeType::RECTANGLE;
            shape.filled = true;
            cc.shape = shape;
        }
        cc.shape->width = width;
        cc.shape->height = height;
        return true;
    };

    component["getColliderEnabled"] = [&registry](Entity entity) -> sol::optional<bool> {
        if (!registry.HasComponent<ColliderComponent>(entity)) {
            return std::nullopt;
        }
        return registry.GetComponent<ColliderComponent>(entity).enabled;
    };

    component["setColliderEnabled"] = [&registry](Entity entity, bool enabled) -> bool {
        if (!registry.HasComponent<ColliderComponent>(entity)) {
            return false;
        }
        registry.GetComponent<ColliderComponent>(entity).enabled = enabled;
        return true;
    };

    component["getColliderTrigger"] = [&registry](Entity entity) -> sol::optional<bool> {
        if (!registry.HasComponent<ColliderComponent>(entity)) {
            return std::nullopt;
        }
        return registry.GetComponent<ColliderComponent>(entity).isTrigger;
    };

    component["setColliderTrigger"] = [&registry](Entity entity, bool isTrigger) -> bool {
        if (!registry.HasComponent<ColliderComponent>(entity)) {
            return false;
        }
        registry.GetComponent<ColliderComponent>(entity).isTrigger = isTrigger;
        return true;
    };

    component["getColliderStatic"] = [&registry](Entity entity) -> sol::optional<bool> {
        if (!registry.HasComponent<ColliderComponent>(entity)) {
            return std::nullopt;
        }
        return registry.GetComponent<ColliderComponent>(entity).isStatic;
    };

    component["setColliderStatic"] = [&registry](Entity entity, bool isStatic) -> bool {
        if (!registry.HasComponent<ColliderComponent>(entity)) {
            return false;
        }
        registry.GetComponent<ColliderComponent>(entity).isStatic = isStatic;
        return true;
    };

    // Polls the CollisionSystem's active contact pairs for this frame.
    // False (empty) when the system is not registered.
    component["isColliding"] = [&registry](Entity entity) -> bool {
        std::shared_ptr<CollisionSystem> system;
        try {
            system = registry.GetSystem<CollisionSystem>();
        } catch (...) {
            return false;
        }
        if (!system) {
            return false;
        }
        for (const auto& [a, b] : system->GetActiveContacts()) {
            if (a == entity || b == entity) {
                return true;
            }
        }
        return false;
    };

    component["getCollisions"] = [&registry](sol::this_state ts, Entity entity) -> sol::table {
        sol::state_view lua(ts);
        sol::table out = lua.create_table();
        std::shared_ptr<CollisionSystem> system;
        try {
            system = registry.GetSystem<CollisionSystem>();
        } catch (...) {
            return out;
        }
        if (!system) {
            return out;
        }
        int i = 1;
        for (const auto& [a, b] : system->GetActiveContacts()) {
            if (a == entity) {
                out[i++] = b;
            } else if (b == entity) {
                out[i++] = a;
            }
        }
        return out;
    };
}

} // namespace Engine::LuaAPI
