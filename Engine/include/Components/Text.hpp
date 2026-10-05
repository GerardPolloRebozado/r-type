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
/**
 * @struct Text
 * @brief Component representing 2D renderable text in the ECS.
 *
 * Holds string content, font reference, font size, character spacing, and tint color.
 * Rendered by @ref TextSystem at the entity's @ref Position.
 */
struct Text {
    /** @brief Content string to display. */
    std::string text = "";

    /** @brief Raylib Font asset used for rendering (default font if texture.id == 0). */
    Font font = {};

    /** @brief Font size in pixels. */
    float fontsize = 10.0f;

    /** @brief Spacing between characters. */
    float spacing = 1.0f;

    /** @brief Tint color of the text. */
    Color color = WHITE;

    /**
     * @brief Constructs a Text component with default font.
     * @param Text The text string to be displayed.
     * @param Fontsize The size of the text in pixels.
     * @param tint The color tint for rendering.
     */
    Text(std::string Text = "", float Fontsize = 10.0f, Color tint = WHITE)
        : text(std::move(Text)), fontsize(Fontsize), color(tint) {}

    /**
     * @brief Constructs a Text component with a custom font and character spacing.
     * @param font The Raylib Font asset to use.
     * @param Text The text string to be displayed.
     * @param Fontsize The size of the text in pixels.
     * @param Spacing The space between characters in pixels.
     * @param tint The color tint for rendering.
     */
    Text(Font font, std::string Text, float Fontsize, float Spacing, Color tint)
        : text(std::move(Text)), font(std::move(font)), fontsize(Fontsize), spacing(Spacing),
          color(tint) {}
};
} // namespace Engine
#endif // G_ING_500_BAR_5_1_RTYPE_1_TEXT_HPP
