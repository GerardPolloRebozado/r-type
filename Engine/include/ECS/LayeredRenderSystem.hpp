#ifndef ENGINE_ECS_LAYEREDRENDERSYSTEM_HPP_
#define ENGINE_ECS_LAYEREDRENDERSYSTEM_HPP_

#include "ECS/System.hpp"

namespace Engine {
class Registry;

/**
 * @class LayeredRenderSystem
 * @brief A system that draws entities one at a time, so @ref RenderScene can mix them
 *        with the entities of other layered systems, sorted by @ref Position::z.
 *
 * Within a pass, entities are drawn by ascending z, then by system registration order,
 * then by entity id.
 */
class LayeredRenderSystem : public System {
  public:
    /**
     * @brief Whether this system draws @p entity in the given pass.
     * @param registry The ECS registry.
     * @param entity The entity to check (it always has a @ref Position).
     * @param screenSpace True for the screen (HUD) pass, false for the world pass.
     */
    virtual bool CanDraw(Registry& registry, Entity entity, bool screenSpace) = 0;

    /**
     * @brief Draws a single entity. Only called when @ref CanDraw returned true.
     * @param registry The ECS registry.
     * @param entity The entity to draw.
     */
    virtual void DrawEntity(Registry& registry, Entity entity) = 0;
};
} // namespace Engine

#endif // ENGINE_ECS_LAYEREDRENDERSYSTEM_HPP_
