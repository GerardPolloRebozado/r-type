#ifndef ENGINE_SYSTEMS_TRANSFORMSYSTEM_HPP_
#define ENGINE_SYSTEMS_TRANSFORMSYSTEM_HPP_
#include "ECS/Registry.hpp"
#include "ECS/System.hpp"

namespace Engine {
/**
 * @class TransformSystem
 * @brief A system that updates the Transform component of entities.
 *
 * Responsible for processing all entities that have a Transform component.
 */
class TransformSystem : public System {
  public:
    /**
     * @brief Updates all entities with a Transform component.
     * @param registry The ECS registry containing the components.
     * @param dt Delta time for framerate independent movement.
     */
    void Update(Registry& registry, float dt);
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_TRANSFORMSYSTEM_HPP_
