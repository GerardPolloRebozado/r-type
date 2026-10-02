#include "Systems/SpriteSystem.hpp"
#include "Components/SpriteComponent.hpp"
#include "Components/Transform.hpp"
#include "ECS/Registry.hpp"

namespace Engine {

void SpriteSystem::Update(Registry& registry, float dt) {
    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Entity entity{i};
        if (registry.HasComponent<Transform>(entity) && registry.HasComponent<SpriteComponent>(entity)) {
            auto& transform = registry.GetComponent<Transform>(entity);
            auto& spriteComp = registry.GetComponent<SpriteComponent>(entity);
            
            if (spriteComp.sprite) {
                spriteComp.sprite->SetPosition({transform.x, transform.y});
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
