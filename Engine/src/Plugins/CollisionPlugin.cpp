#include "Plugins/CollisionPlugin.hpp"

namespace Engine {
void CollisionPlugin::Init(Registry& registry) {
    registry.RegisterComponent<Collider>();
    registry.RegisterComponent<CollisionEvents>();
    registry.RegisterSystem<CollisionSystem>();
}

void CollisionPlugin::AttachToEntity(Registry& registry, Entity entity, float width,
                           float height) {
    registry.AddComponent(entity, Collider{width, height});
    registry.AddComponent(entity, CollisionEvents{});
}
}
