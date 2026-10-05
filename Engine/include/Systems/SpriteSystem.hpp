#ifndef ENGINE_SYSTEMS_SPRITESYSTEM_HPP_
#define ENGINE_SYSTEMS_SPRITESYSTEM_HPP_

#include "ECS/System.hpp"

namespace Engine {
class Registry;

/**
 * @class SpriteSystem
 * @brief System that draws SpriteComponent based on Position.
 */
class SpriteSystem : public System {
  public:
    /**
     * @brief Renders all entities that have both a Position and a SpriteComponent,
     *        at Position plus the sprite's offset.
     * @param registry The ECS registry containing the entities.
     */
    void Render(Registry& registry) override;
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_SPRITESYSTEM_HPP_
