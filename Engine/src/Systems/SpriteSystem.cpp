#include "Systems/SpriteSystem.hpp"
#include "Components/Position.hpp"
#include "Components/SpriteComponent.hpp"
#include "ECS/Registry.hpp"

namespace Engine {

void SpriteSystem::Update(Registry& registry, float dt) {
    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Entity entity{i};
        if (registry.HasComponent<Position>(entity) &&
            registry.HasComponent<SpriteComponent>(entity)) {
            auto& position = registry.GetComponent<Position>(entity);
            auto& spriteComp = registry.GetComponent<SpriteComponent>(entity);

            if (spriteComp.sprite) {
                spriteComp.sprite->SetPosition({position.x, position.y});
            }
        }
    }
}

void SpriteSystem::Render(Registry& registry) {
    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Entity entity{i};
        if (registry.HasComponent<SpriteComponent>(entity)) {
            auto& spriteComp = registry.GetComponent<SpriteComponent>(entity);

            if (spriteComp.sprite) {
                spriteComp.sprite->Draw();
            }
        }
    }
}

} // namespace Engine
