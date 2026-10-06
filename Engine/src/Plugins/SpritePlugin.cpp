#include "Plugins/SpritePlugin.hpp"
#include "Assets/AssetManager.hpp"

namespace Engine {
void SpritePlugin::Init(Registry& registry) {
    if (!registry.HasResource<AssetManager>()) {
        registry.AddResource<AssetManager>();
    }
    registry.RegisterComponent<Position>();
    registry.RegisterComponent<SpriteComponent>();
    registry.RegisterSystem<SpriteSystem>();
}

void SpritePlugin::AttachToEntity(Registry& registry, Entity entity,
                                  const SpriteComponent& sprite) {
    registry.AddComponent(entity, sprite);
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent(entity, Position{});
    }
}
} // namespace Engine
