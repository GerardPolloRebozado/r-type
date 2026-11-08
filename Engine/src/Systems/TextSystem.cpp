/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** TextSystem.cpp
*/
#include "../../include/Systems/TextSystem.hpp"

#include "Assets/ProjectPaths.hpp"
#include "Components/Opacity.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ScreenSpace.hpp"
#include "Components/Text.hpp"
#include "ECS/Registry.hpp"
#include "Plugins/OpacityPlugin.hpp"
#include <filesystem>

namespace Engine {

bool TextSystem::CanDraw(Registry& registry, Entity entity, bool screenSpace) {
    return registry.HasComponent<Text>(entity) &&
           registry.HasComponent<ScreenSpace>(entity) == screenSpace;
}

void TextSystem::DrawEntity(Registry& registry, Entity entity) {
    auto& txt = registry.RequireComponent<Text>(entity);
    const auto& pos = registry.RequireComponent<Position>(entity);
    float rotation = 0.0f;
    if (registry.HasComponent<Rotation>(entity)) {
        rotation = registry.RequireComponent<Rotation>(entity).angle;
    }

    if (txt.font.texture.id == 0 && !txt.fontPath.empty()) {
        const std::filesystem::path resolved = ResolveAssetPath(registry, txt.fontPath);
        if (std::filesystem::exists(resolved)) {
            txt.font = LoadFont(resolved.string().c_str());
        }
    }

    Font font = (txt.font.texture.id == 0) ? GetFontDefault() : txt.font;
    // children without their own Opacity still
    // inherit faded ancestors (1.0 when fully opaque = no change).
    Color drawColor = txt.color;
    drawColor.a = static_cast<unsigned char>(static_cast<float>(drawColor.a) *
                                             OpacityPlugin::GetEffectiveAlpha(registry, entity));
    DrawTextPro(font, txt.text.c_str(), {pos.x, pos.y}, {0.0f, 0.0f}, rotation, txt.fontsize,
                txt.spacing, drawColor);
}

} // namespace Engine