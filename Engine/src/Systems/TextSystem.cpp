/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** TextSystem.cpp
*/
#include "../../include/Systems/TextSystem.hpp"

#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ScreenSpace.hpp"
#include "Components/Text.hpp"
#include "ECS/Registry.hpp"

namespace Engine {
bool TextSystem::CanDraw(Registry& registry, Entity entity, bool screenSpace) {
    return registry.HasComponent<Text>(entity) &&
           registry.HasComponent<ScreenSpace>(entity) == screenSpace;
}

void TextSystem::DrawEntity(Registry& registry, Entity entity) {
    const auto& txt = registry.GetComponent<Text>(entity);
    const auto& pos = registry.GetComponent<Position>(entity);
    float rotation = 0.0f;
    if (registry.HasComponent<Rotation>(entity)) {
        rotation = registry.GetComponent<Rotation>(entity).angle;
    }

    Font font = (txt.font.texture.id == 0) ? GetFontDefault() : txt.font;
    DrawTextPro(font, txt.text.c_str(), {pos.x, pos.y}, {0.0f, 0.0f}, rotation, txt.fontsize,
                txt.spacing, txt.color);
}
} // namespace Engine
