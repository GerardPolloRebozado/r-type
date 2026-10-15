#include "Systems/SpriteSystem.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/Opacity.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ScreenSpace.hpp"
#include "Components/SpriteComponent.hpp"
#include "ECS/Registry.hpp"
#include "Plugins/OpacityPlugin.hpp"

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

            if (!spriteComp.sprite && !spriteComp.assetPath.empty()) {
                if (registry.HasResource<AssetManager>()) {
                    auto& am = registry.GetResource<AssetManager>();
                    try {
                        am.LoadSprite(spriteComp.assetPath, spriteComp.assetPath);
                    } catch (...) {
                    }
                    try {
                        spriteComp.sprite = am.GetSprite(spriteComp.assetPath);
                    } catch (...) {
                    }
                }
            }

            if (spriteComp.sprite) {
                if (spriteComp.scaleX > 0.0f && spriteComp.scaleY > 0.0f) {
                    spriteComp.sprite->SetScale({spriteComp.scaleX, spriteComp.scaleY});
                }
                if (registry.HasComponent<Rotation>(entity)) {
                    spriteComp.sprite->SetRotation(registry.GetComponent<Rotation>(entity).angle);
                }
                raylib::Color tint = raylib::Color::White();
                // children without their own Opacity still
                // inherit faded ancestors (effective alpha is 1.0 when the
                // chain is fully opaque, i.e. no visual change).
                tint.a = static_cast<unsigned char>(
                    255.0f * OpacityPlugin::GetEffectiveAlpha(registry, entity));
                spriteComp.sprite->Draw(
                    {position.x + spriteComp.offsetX, position.y + spriteComp.offsetY}, tint);
            }
        }
    }
}

} // namespace Engine
