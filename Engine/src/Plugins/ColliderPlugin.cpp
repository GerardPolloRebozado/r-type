#include "Plugins/ColliderPlugin.hpp"
#include "Components/Name.hpp"
#include "Components/Position.hpp"
#include "Logging/Logger.hpp"
#include <string>
#include <unordered_set>

namespace Engine {

namespace {

void ReportMissingShape(Registry& registry, Entity entity) {
    static std::unordered_set<Entity> warned;
    std::string label;

    if (!warned.insert(entity).second) {
        return;
    }
    label = "entity " + std::to_string(entity);
    if (registry.IsComponentRegistered<Name>() && registry.HasComponent<Name>(entity)) {
        const std::string& value = registry.GetComponent<Name>(entity).value;
        if (!value.empty()) {
            label = value;
        }
    }
    log_user_error("Collider on " + label +
                   " has no shape. Add a Shape component or set a collider shape.");
}

} // namespace

void ColliderPlugin::Init(Registry& registry) {
    registry.RegisterComponent<Position>();
    registry.RegisterComponent<ColliderComponent>();
    registry.RegisterComponent<CollisionEvents>();
    // Shape lookup must not throw when a collider falls back to the entity shape.
    registry.RegisterComponent<ShapeComponent>();
    registry.RegisterSystem<CollisionSystem>();
}

void ColliderPlugin::AttachToEntity(Registry& registry, const Entity entity,
                                    const std::optional<ShapeComponent>& shape, const bool enabled,
                                    const bool isTrigger) {
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent(entity, Position(0.0f, 0.0f));
    }
    std::optional<ShapeComponent> colliderShape = shape;
    if (!colliderShape.has_value() && !EntityHasShape(registry, entity)) {
        colliderShape = ShapeComponent{};
    }
    registry.AddComponent(entity, ColliderComponent::Create(colliderShape, enabled, isTrigger));
    if (!registry.HasComponent<CollisionEvents>(entity)) {
        registry.AddComponent(entity, CollisionEvents{});
    }
}

void ColliderPlugin::AttachToEntity(Registry& registry, const Entity entity,
                                    const ColliderComponent& collider) {
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent(entity, Position(0.0f, 0.0f));
    }
    ColliderComponent col = collider;
    if (!col.shape.has_value() && !EntityHasShape(registry, entity)) {
        col.shape = ShapeComponent{};
    }
    registry.AddComponent(entity, col);
    if (!registry.HasComponent<CollisionEvents>(entity)) {
        registry.AddComponent(entity, CollisionEvents{});
    }
}

bool ColliderPlugin::EntityHasShape(Registry& registry, Entity entity) {
    return registry.IsComponentRegistered<ShapeComponent>() &&
           registry.HasComponent<ShapeComponent>(entity);
}

std::optional<ShapeComponent> ColliderPlugin::ResolveShape(Registry& registry, Entity entity,
                                                           const ColliderComponent& collider) {
    if (collider.shape.has_value()) {
        return collider.shape;
    }
    if (EntityHasShape(registry, entity)) {
        return registry.GetComponent<ShapeComponent>(entity);
    }
    ReportMissingShape(registry, entity);
    return std::nullopt;
}

} // namespace Engine