/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** Text.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_TEXT_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_TEXT_HPP
#include <string>
#include <utility>

#include "raylib-cpp.hpp"

namespace Engine {
struct Text {
    std::string text = "";
    Font font = {};
    float fontsize = 10.0f;
    float spacing = 1.0f;
    Color color = WHITE;
    /**
     * @brief Constructor for Text.
     * @param Text The text that is going to be displayed
     * @param Fontsize The size of the text
     * @param tint The color that is wanted on the text
     */
    Text(std::string Text = "", float Fontsize = 10.0, Color tint = WHITE)
        : text(std::move(Text)), fontsize(Fontsize), color(tint) {}
    /**
     * @brief Constructor for Text.
     * @param font The type of font
     * @param Text The text that is going to be displayed
     * @param Fontsize The size of the text
     * @param Spacing The space between the characters
     * @param tint The color that is wanted on the text
     */
    Text(Font font, std::string Text, float Fontsize, float Spacing, Color tint)
        : text(std::move(Text)), font(std::move(font)), fontsize(Fontsize), spacing(Spacing),
          color(tint) {}
};
} // namespace Engine
#endif // G_ING_500_BAR_5_1_RTYPE_1_TEXT_HPP
