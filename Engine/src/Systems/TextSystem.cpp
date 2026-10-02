/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** TextSystem.cpp
*/
#include "../../include/Systems/TextSystem.hpp"

#include "Components/Text.hpp"
#include "Components/Position.hpp"
#include "ECS/Registry.hpp"

namespace Engine {
    void TextSystem::Update(Registry&registry, float dt)
    {
        for (const auto& entity : m_Entities) {
            const auto& txt =  registry.GetComponent<Text>(entity);
            const auto& pos = registry.GetComponent<Position>(entity);
            if (txt.font.texture.id == 0) {
                DrawText(txt.text.c_str(), pos.x, pos.y, txt.fontsize, txt
                    .color);
            } else
                DrawTextEx(txt.font, txt.text.c_str(), {pos.x, pos.y}, txt.fontsize, txt.spacing, txt.color);
        }
    }
} // Engine