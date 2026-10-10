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
    if (!registry.HasComponent<ShapeComponent>(entity)) {
        registry.AddComponent(entity, ShapeComponent{});
    }
    registry.AddComponent(entity, ColliderComponent{shape, enabled, isTrigger});
    if (!registry.HasComponent<CollisionEvents>(entity)) {
        registry.AddComponent(entity, CollisionEvents{});
    }
}

void ColliderPlugin::AttachToEntity(Registry& registry, const Entity entity,
                                    const ColliderComponent& collider) {
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent(entity, Position{0.0f, 0.0f});
    }
    if (!registry.HasComponent<ShapeComponent>(entity)) {
        registry.AddComponent(entity, ShapeComponent{});
    }
    registry.AddComponent(entity, collider);
    if (!registry.HasComponent<CollisionEvents>(entity)) {
        registry.AddComponent(entity, CollisionEvents{});
    }
}

} // namespace Engine