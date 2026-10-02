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
    std::shared_ptr<Sprite> sprite;

    SpriteComponent(const std::string& path) {
        sprite = std::make_shared<Sprite>(path);
    }

    SpriteComponent(std::shared_ptr<Sprite> spritePtr = nullptr) 
        : sprite(spritePtr) {}
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_SPRITECOMPONENT_HPP_
