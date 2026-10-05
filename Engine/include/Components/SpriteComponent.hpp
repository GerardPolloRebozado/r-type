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
     * @brief Shared pointer to the underlying Sprite.
     */
    std::shared_ptr<Sprite> sprite;

    /** @brief Horizontal offset from Position.x to the sprite's top-left corner. */
    float offsetX;

    /** @brief Vertical offset from Position.y to the sprite's top-left corner. */
    float offsetY;

    /**
     * @brief Constructs a SpriteComponent from a file path.
     * @param path The file path to the image file.
     * @param offsetX Offset from entity position on X.
     * @param offsetY Offset from entity position on Y.
     */
    SpriteComponent(const std::string& path, float offsetX = 0.0f, float offsetY = 0.0f)
        : sprite(std::make_shared<Sprite>(path)), offsetX(offsetX), offsetY(offsetY) {}

    /**
     * @brief Constructs a SpriteComponent with an optional existing sprite pointer.
     * @param spritePtr A shared pointer to an existing Sprite. Defaults to nullptr.
     * @param offsetX Offset from entity position on X.
     * @param offsetY Offset from entity position on Y.
     */
    SpriteComponent(std::shared_ptr<Sprite> spritePtr = nullptr, float offsetX = 0.0f,
                    float offsetY = 0.0f)
        : sprite(spritePtr), offsetX(offsetX), offsetY(offsetY) {}
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_SPRITECOMPONENT_HPP_
