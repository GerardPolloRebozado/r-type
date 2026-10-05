#ifndef ENGINE_SYSTEMS_POSITIONSYSTEM_HPP_
#define ENGINE_SYSTEMS_POSITIONSYSTEM_HPP_
#include "ECS/Registry.hpp"
#include "ECS/System.hpp"

namespace Engine {
/**
 * @class PositionSystem
 * @brief A system that updates the Position component of entities.
 *
 * Stores the previous coordinates before the current frame modifies position.
 */
class PositionSystem : public System {
  public:
    /**
     * @brief Updates previous position for entities in @ref System::m_Entities.
     * @param registry The ECS registry containing the components.
     * @param dt Delta time for framerate independent movement.
     */
    void Update(Registry& registry, float dt);
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_POSITIONSYSTEM_HPP_
