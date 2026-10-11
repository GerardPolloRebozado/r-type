#include "Plugins/ColliderPlugin.hpp"
#include "Components/Position.hpp"

namespace Engine {
void ColliderPlugin::Init(Registry& registry) {
    registry.RegisterComponent<Position>();
    registry.RegisterComponent<ColliderComponent>();
    registry.RegisterComponent<CollisionEvents>();
    registry.RegisterSystem<CollisionSystem>();
}

void ColliderPlugin::AttachToEntity(Registry& registry, const Entity entity,
                                    const std::optional<ShapeComponent>& shape, const bool enabled,
                                    const bool isTrigger) {
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent(entity, Position{0.0f, 0.0f});
    }
    std::optional<ShapeComponent> colliderShape = shape;
    if (!colliderShape.has_value() && !registry.HasComponent<ShapeComponent>(entity)) {
        colliderShape = ShapeComponent{};
    }
    registry.AddComponent(entity, ColliderComponent{colliderShape, enabled, isTrigger});
    if (!registry.HasComponent<CollisionEvents>(entity)) {
        registry.AddComponent(entity, CollisionEvents{});
    }
}

void ColliderPlugin::AttachToEntity(Registry& registry, const Entity entity,
                                    const ColliderComponent& collider) {
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent(entity, Position{0.0f, 0.0f});
    }
    ColliderComponent col = collider;
    if (!col.shape.has_value() && !registry.HasComponent<ShapeComponent>(entity)) {
        col.shape = ShapeComponent{};
    }
    registry.AddComponent(entity, col);
    if (!registry.HasComponent<CollisionEvents>(entity)) {
        registry.AddComponent(entity, CollisionEvents{});
    }
}

} // namespace Engine