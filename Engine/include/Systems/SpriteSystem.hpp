#ifndef ENGINE_SYSTEMS_SPRITESYSTEM_HPP_
#define ENGINE_SYSTEMS_SPRITESYSTEM_HPP_

#include "ECS/LayeredRenderSystem.hpp"

namespace Engine {
class Registry;

/**
 * @class SpriteSystem
 * @brief System that draws SpriteComponent based on Position.
 */
class SpriteSystem : public LayeredRenderSystem {
  public:
    /**
     * @brief True if @p entity has a SpriteComponent and belongs to this pass
     *        (entities tagged with @ref ScreenSpace go in the screen pass).
     */
    bool CanDraw(Registry& registry, Entity entity, bool screenSpace) override;

    /**
     * @brief Draws the entity's sprite at Position plus the sprite's offset.
     */
    void DrawEntity(Registry& registry, Entity entity) override;
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_SPRITESYSTEM_HPP_
