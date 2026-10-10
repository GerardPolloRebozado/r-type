#include "Plugins/CollisionPlugin.hpp"
#include "Components/Position.hpp"

namespace Engine {
void CollisionPlugin::Init(Registry& registry) {
    registry.RegisterComponent<Position>();
    registry.RegisterComponent<Collider>();
    registry.RegisterComponent<CollisionEvents>();
    registry.RegisterSystem<CollisionSystem>();
}

void CollisionPlugin::AttachToEntity(Registry& registry, const Entity entity,
                                     const std::optional<ShapeComponent>& shape, const bool enabled,
                                     const bool isTrigger) {
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent(entity, Position{0.0f, 0.0f});
    }
    registry.AddComponent(entity, Collider{shape, enabled, isTrigger});
    if (!registry.HasComponent<CollisionEvents>(entity)) {
        registry.AddComponent(entity, CollisionEvents{});
    }
}

} // namespace Engine