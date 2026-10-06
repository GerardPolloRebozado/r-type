#include "Systems/SpriteSystem.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/Position.hpp"
#include "Components/SpriteComponent.hpp"
#include "ECS/Registry.hpp"

namespace Engine {

void SpriteSystem::Render(Registry& registry) {
    if (!registry.HasResource<AssetManager>()) {
        return;
    }
    auto& assetManager = registry.GetResource<AssetManager>();

    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Entity entity{i};
        if (registry.HasComponent<Position>(entity) &&
            registry.HasComponent<SpriteComponent>(entity)) {
            auto& position = registry.GetComponent<Position>(entity);
            auto& spriteComp = registry.GetComponent<SpriteComponent>(entity);

            if (!spriteComp.spriteAlias.empty()) {
                try {
                    auto& sprite = assetManager.GetSprite(spriteComp.spriteAlias);
                    sprite.Draw({position.x + spriteComp.offsetX, position.y + spriteComp.offsetY});
                } catch (...) {
                }
            }
        }
    }
}

} // namespace Engine
