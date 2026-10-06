#ifndef ENGINE_COMPONENTS_SPRITECOMPONENT_HPP_
#define ENGINE_COMPONENTS_SPRITECOMPONENT_HPP_

#include "Assets/Sprites.hpp"
#include <memory>
#include <string>

namespace Engine {
/**
 * @struct SpriteComponent
 * @brief Component that holds a Sprite to be rendered.
 *
 * World-space position comes from the @ref Position on the same entity.
 * The sprite is drawn at (Position.x + offsetX, Position.y + offsetY).
 */
struct SpriteComponent {
    /**
     * @brief Alias for the sprite in the AssetManager.
     */
    std::string spriteAlias;

    /** @brief Horizontal offset from Position.x to the sprite's top-left corner. */
    float offsetX;

    /** @brief Vertical offset from Position.y to the sprite's top-left corner. */
    float offsetY;

    /**
     * @brief Constructs a SpriteComponent with an alias.
     * @param alias The alias of the sprite in AssetManager.
     * @param offsetX Offset from entity position on X.
     * @param offsetY Offset from entity position on Y.
     */
    SpriteComponent(const std::string& alias = "", float offsetX = 0.0f, float offsetY = 0.0f)
        : spriteAlias(alias), offsetX(offsetX), offsetY(offsetY) {}
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_SPRITECOMPONENT_HPP_
