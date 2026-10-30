#ifndef ENGINE_SYSTEMS_ANIMATIONSYSTEM_HPP_
#define ENGINE_SYSTEMS_ANIMATIONSYSTEM_HPP_

#include "ECS/LayeredRenderSystem.hpp"

namespace Engine {
class Registry;

/**
 * @class AnimationSystem
 * @brief System that updates animation frames and renders the current frame based on Position.
 */
class AnimationSystem : public LayeredRenderSystem {
  public:
    /**
     * @brief Advances animation timers and frame indices for all entities with Animation.
     */
    void Update(Registry& registry, float dt) override;

    /**
     * @brief True if @p entity has an Animation component and belongs to this pass.
     */
    bool CanDraw(Registry& registry, Entity entity, bool screenSpace) override;

    /**
     * @brief Draws the entity's current animation frame at Position plus offset.
     */
    void DrawEntity(Registry& registry, Entity entity) override;
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_ANIMATIONSYSTEM_HPP_
