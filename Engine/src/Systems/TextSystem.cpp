/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** TextSystem.cpp
*/
#include "../../include/Systems/TextSystem.hpp"

#include "Components/Opacity.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ScreenSpace.hpp"
#include "Components/Text.hpp"
#include "ECS/Registry.hpp"
#include "Plugins/OpacityPlugin.hpp"

namespace Engine {
void TextSystem::Render(Registry& registry) {
    DrawTexts(registry, false);
}

void TextSystem::RenderScreen(Registry& registry) {
    DrawTexts(registry, true);
}

void TextSystem::DrawTexts(Registry& registry, bool screenSpace) {
    for (const auto& [entity, txt] : *registry.GetComponentArray<Text>()) {
        if (registry.HasComponent<Position>(entity) &&
            registry.HasComponent<ScreenSpace>(entity) == screenSpace) {
            const auto& pos = registry.GetComponent<Position>(entity);
            float rotation = 0.0f;
            if (registry.HasComponent<Rotation>(entity)) {
                rotation = registry.GetComponent<Rotation>(entity).angle;
            }

            Font font = (txt.font.texture.id == 0) ? GetFontDefault() : txt.font;
            // children without their own Opacity still
            // inherit faded ancestors (1.0 when fully opaque = no change).
            Color drawColor = txt.color;
            drawColor.a =
                static_cast<unsigned char>(static_cast<float>(drawColor.a) *
                                           OpacityPlugin::GetEffectiveAlpha(registry, entity));
            DrawTextPro(font, txt.text.c_str(), {pos.x, pos.y}, {0.0f, 0.0f}, rotation,
                        txt.fontsize, txt.spacing, drawColor);
        }
    }
}
} // namespace Engine
