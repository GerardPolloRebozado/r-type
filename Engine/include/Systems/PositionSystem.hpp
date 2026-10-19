#ifndef ENGINE_SYSTEMS_POSITIONSYSTEM_HPP_
#define ENGINE_SYSTEMS_POSITIONSYSTEM_HPP_
#include "ECS/Registry.hpp"
#include "ECS/System.hpp"

namespace Engine {
/**
 * @class PositionSystem
 * @brief A system that updates the Position component of entities.
 *
 * Snapshots previous coordinates for EVERY Position holder (not just
 * @ref System::m_Entities, which nothing fills automatically) so
 * @ref HierarchySystem always sees a fresh baseline. Must run after
 * HierarchySystem in the update order (see HierarchyPlugin).
 */
class PositionSystem : public System {
  public:
    /**
     * @brief Snapshots prevX/prevY/prevZ (and the moved flag) for all Positions.
     * @param registry The ECS registry containing the components.
     * @param dt Delta time for framerate independent movement.
     */
    void Update(Registry& registry, float dt);
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_POSITIONSYSTEM_HPP_
