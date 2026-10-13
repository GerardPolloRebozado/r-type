#include "Scripting/LuaAPI/Plugins/ColliderAPI.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/ShapeComponent.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>

namespace Engine::LuaAPI {

void registerCollider(sol::state_view lua, Registry& registry) {
    sol::table component = Internal::ensureNamespace(lua, "Component");

    component["addBoxCollider"] = [&registry](Entity entity, float width, float height,
                                              bool isTrigger) {
        Internal::ensureTransform(registry, entity, false);
        ColliderComponent cc;
        cc.enabled = true;
        cc.isTrigger = isTrigger;
        ShapeComponent shape;
        shape.type = ShapeType::RECTANGLE;
        shape.width = width;
        shape.height = height;
        shape.filled = true;
        cc.shape = shape;
        registry.AddComponent<ColliderComponent>(entity, cc);
    };
}

} // namespace Engine::LuaAPI
