#include "Systems/SpriteSystem.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ScreenSpace.hpp"
#include "Components/SpriteComponent.hpp"
#include "ECS/Registry.hpp"

namespace Engine {

void SpriteSystem::Render(Registry& registry) {
    DrawSprites(registry, false);
}

void SpriteSystem::RenderScreen(Registry& registry) {
    DrawSprites(registry, true);
}

void SpriteSystem::DrawSprites(Registry& registry, bool screenSpace) {
    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Entity entity{i};
        if (registry.HasComponent<Position>(entity) &&
            registry.HasComponent<SpriteComponent>(entity) &&
            registry.HasComponent<ScreenSpace>(entity) == screenSpace) {
            auto& position = registry.GetComponent<Position>(entity);
            auto& spriteComp = registry.GetComponent<SpriteComponent>(entity);

            if (spriteComp.sprite) {
                if (spriteComp.scaleX > 0.0f && spriteComp.scaleY > 0.0f) {
                    spriteComp.sprite->SetScale({spriteComp.scaleX, spriteComp.scaleY});
                }
                if (registry.HasComponent<Rotation>(entity)) {
                    spriteComp.sprite->SetRotation(registry.GetComponent<Rotation>(entity).angle);
                }
                spriteComp.sprite->Draw(
                    {position.x + spriteComp.offsetX, position.y + spriteComp.offsetY});
            }
        }
    }
}

} // namespace Engine
