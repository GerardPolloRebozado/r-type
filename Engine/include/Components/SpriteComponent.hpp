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

    /** @brief The file path or alias used to load the sprite from AssetManager. */
    std::string assetPath;

    /** @brief Horizontal offset from Position.x to the sprite's top-left corner. */
    float offsetX;

    /** @brief Vertical offset from Position.y to the sprite's top-left corner. */
    float offsetY;

    /** @brief Horizontal scale multiplier of the sprite. */
    float scaleX = 1.0f;

    /** @brief Vertical scale multiplier of the sprite. */
    float scaleY = 1.0f;

    /**
     * @brief Constructs a SpriteComponent with an optional existing sprite pointer.
     * @param spritePtr A shared pointer to an existing Sprite. Defaults to nullptr.
     * @param path The file path or alias of the asset.
     * @param offsetX Offset from entity position on X.
     * @param offsetY Offset from entity position on Y.
     * @param scaleX Horizontal scale. Defaults to 1.0f.
     * @param scaleY Vertical scale. Defaults to 1.0f.
     */
    SpriteComponent(std::shared_ptr<Sprite> spritePtr = nullptr, const std::string& path = "",
                    float offsetX = 0.0f, float offsetY = 0.0f, float scaleX = 1.0f,
                    float scaleY = 1.0f)
        : sprite(spritePtr), assetPath(path), offsetX(offsetX), offsetY(offsetY), scaleX(scaleX),
          scaleY(scaleY) {}
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_SPRITECOMPONENT_HPP_
