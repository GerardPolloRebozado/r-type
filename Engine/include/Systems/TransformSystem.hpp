#ifndef ENGINE_SYSTEMS_TRANSFORMSYSTEM_HPP_
#define ENGINE_SYSTEMS_TRANSFORMSYSTEM_HPP_
#include "ECS/Registry.hpp"
#include "ECS/System.hpp"

namespace Engine {
/**
 * @class TransformSystem
 * @brief A system that updates the Position component of entities.
 *
 * Stores the previous coordinates before the current frame modifies position.
 */
class TransformSystem : public System {
  public:
    /**
     * @brief Updates previous position for entities in @ref System::m_Entities.
     * @param registry The ECS registry containing the components.
     * @param dt Delta time for framerate independent movement.
     */
    void Update(Registry& registry, float dt);
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_TRANSFORMSYSTEM_HPP_
