#ifndef ENGINE_COMPONENTS_SPRITECOMPONENT_HPP_
#define ENGINE_COMPONENTS_SPRITECOMPONENT_HPP_

#include "Assets/Sprites.hpp"
#include <memory>
#include <string>

namespace Engine {
/**
 * @struct SpriteComponent
 * @brief Component that holds a Sprite to be rendered.
 */
struct SpriteComponent {
    /**
     * @brief Shared pointer to the underlying Sprite.
     */
    std::shared_ptr<Sprite> sprite;

    /**
     * @brief Constructs a SpriteComponent from a file path.
     * @param path The file path to the image file.
     */
    SpriteComponent(const std::string& path) {
        sprite = std::make_shared<Sprite>(path);
    }

    /**
     * @brief Constructs a SpriteComponent with an optional existing sprite pointer.
     * @param spritePtr A shared pointer to an existing Sprite. Defaults to nullptr.
     */
    SpriteComponent(std::shared_ptr<Sprite> spritePtr = nullptr) 
        : sprite(spritePtr) {}
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_SPRITECOMPONENT_HPP_
