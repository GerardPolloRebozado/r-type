/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** TextSystem.cpp
*/
#include "../../include/Systems/TextSystem.hpp"

#include "Components/Position.hpp"
#include "Components/Text.hpp"
#include "ECS/Registry.hpp"

namespace Engine {
void TextSystem::Render(Registry& registry) {
    for (const auto& [entity, txt] : *registry.GetComponentArray<Text>()) {
        if (registry.HasComponent<Position>(entity)) {
            const auto& pos = registry.GetComponent<Position>(entity);
            if (txt.font.texture.id == 0) {
                DrawText(txt.text.c_str(), pos.x, pos.y, txt.fontsize, txt.color);
            } else {
                DrawTextEx(txt.font, txt.text.c_str(), {pos.x, pos.y}, txt.fontsize, txt.spacing,
                           txt.color);
            }
        }
    }
}
} // namespace Engine
