#ifndef ENGINE_SYSTEMS_SHAPESYSTEM_HPP_
#define ENGINE_SYSTEMS_SHAPESYSTEM_HPP_

#include "ECS/System.hpp"

namespace Engine {
class Registry;

/**
 * @brief System that renders ShapeComponent based on Position.
 */
class ShapeSystem : public System {
  public:
    /**
     * @brief Renders all entities that have both a Position and a ShapeComponent.
     * @param registry The ECS registry containing the entities.
     */
    void Render(Registry& registry) override;
};

} // namespace Engine

#endif // ENGINE_SYSTEMS_SHAPESYSTEM_HPP_
