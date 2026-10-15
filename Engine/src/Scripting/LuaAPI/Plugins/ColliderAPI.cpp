#include "Scripting/LuaAPI/Plugins/ColliderAPI.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/CollisionEvents.hpp"
#include "Components/ShapeComponent.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>

namespace Engine::LuaAPI {

void registerCollider(sol::state_view lua, Registry& registry) {
    sol::table component = Internal::ensureNamespace(lua, "Component");

    auto addBoxCollider = [&registry](Entity entity, float width, float height, bool isTrigger,
                                      bool isStatic) {
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
    };

    component["addBoxCollider"] = sol::overload(
        [&addBoxCollider](Entity entity, float width, float height) {
            addBoxCollider(entity, width, height, false, false);
        },
        [&addBoxCollider](Entity entity, float width, float height, bool isTrigger) {
            addBoxCollider(entity, width, height, isTrigger, false);
        },
        [&addBoxCollider](Entity entity, float width, float height, bool isTrigger, bool isStatic) {
            addBoxCollider(entity, width, height, isTrigger, isStatic);
        });
}

} // namespace Engine::LuaAPI