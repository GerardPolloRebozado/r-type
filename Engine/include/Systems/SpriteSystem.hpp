#ifndef ENGINE_SYSTEMS_SPRITESYSTEM_HPP_
#define ENGINE_SYSTEMS_SPRITESYSTEM_HPP_

#include "ECS/System.hpp"

namespace Engine {
class Registry;

/**
 * @class SpriteSystem
 * @brief System that updates and draws SpriteComponent based on Position.
 */
class SpriteSystem : public System {
public:
    /**
     * @brief Updates the sprite positions based on their position components.
     * @param registry The ECS registry containing the entities.
     * @param dt The delta time since the last frame.
     */
    void Update(Registry& registry, float dt) override;

    /**
     * @brief Renders all entities that have a SpriteComponent.
     * @param registry The ECS registry containing the entities.
     */
    void Render(Registry& registry) override;
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_SPRITESYSTEM_HPP_
