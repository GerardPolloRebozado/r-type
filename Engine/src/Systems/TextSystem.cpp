/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** TextSystem.cpp
*/
#include "../../include/Systems/TextSystem.hpp"

#include "Components/Text.hpp"
#include "Components/Transform.hpp"
#include "ECS/Registry.hpp"

namespace Engine {
    void TextSystem::Update(Registry &registry, float dt) {
        for (const auto &[entity, text]: *registry.GetComponentArray<Text>()) {
            const auto &txt = registry.GetComponent<Text>(entity);
            const auto &pos = registry.GetComponent<Transform>(entity);
            if (txt.font.texture.id == 0) {
                DrawText(txt.text.c_str(), pos.x, pos.y, txt.fontsize, txt
                         .color);
            } else
                DrawTextEx(txt.font, txt.text.c_str(), {.x = pos.x, .y = pos.y}, txt.fontsize, txt.spacing, txt.color);
        }
    }
} // Engine
