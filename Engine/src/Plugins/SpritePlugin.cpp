#include "Plugins/SpritePlugin.hpp"

namespace Engine {
void SpritePlugin::Init(Registry& registry) {
    registry.RegisterComponent<Position>();
    registry.RegisterComponent<SpriteComponent>();
    registry.RegisterSystem<SpriteSystem>();
}

void SpritePlugin::AttachToEntity(Registry& registry, Entity entity, const SpriteComponent& sprite) {
    registry.AddComponent(entity, sprite);
    registry.AddComponent(entity, Position{});
}
}
