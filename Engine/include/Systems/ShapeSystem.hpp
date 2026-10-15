#ifndef ENGINE_SYSTEMS_SHAPESYSTEM_HPP_
#define ENGINE_SYSTEMS_SHAPESYSTEM_HPP_

#include "ECS/LayeredRenderSystem.hpp"

namespace Engine {
class Registry;

/**
 * @brief System that renders ShapeComponent based on Position.
 */
class ShapeSystem : public LayeredRenderSystem {
  public:
    /**
     * @brief True if @p entity has a ShapeComponent and belongs to this pass
     *        (entities tagged with @ref ScreenSpace go in the screen pass).
     */
    bool CanDraw(Registry& registry, Entity entity, bool screenSpace) override;

    /**
     * @brief Draws the entity's shape at its Position.
     */
    void DrawEntity(Registry& registry, Entity entity) override;
};

} // namespace Engine

#endif // ENGINE_SYSTEMS_SHAPESYSTEM_HPP_
