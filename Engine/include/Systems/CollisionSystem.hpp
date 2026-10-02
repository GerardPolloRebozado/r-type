#ifndef ENGINE_SYSTEMS_COLLISIONSYSTEM_HPP_
#define ENGINE_SYSTEMS_COLLISIONSYSTEM_HPP_

#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "ECS/System.hpp"
#include <set>
#include <utility>
#include <vector>

namespace Engine {
/**
 * @class CollisionSystem
 * @brief Detects AABB overlaps each frame and fires enter/exit events.
 *
 * Processes entities listed in @ref System::m_Entities that have both
 * @ref Position and @ref Collider. Optional @ref CollisionEvents on an entity
 * receive callbacks when contacts start or stop.
 *
 * Each frame, entities with @ref Position::moved set are compared against every
 * other registered collider (two-pass loop). Enter/exit events fire via
 * @ref CollisionEvents.
 */
class CollisionSystem : public System {
  public:
    /**
     * @brief Runs overlap detection and updates contact state for this frame.
     * @param registry ECS registry with positions and colliders.
     * @param dt Delta time (unused for pure overlap tests; kept for @ref System API).
     */
    void Update(Registry& registry, float dt) override;

  private:
    using EntityPair = std::pair<Entity, Entity>;

    /**
     * @brief Returns a canonical pair (minId, maxId) so (A,B) and (B,A) match.
     * @param a First entity id.
     * @param b Second entity id.
     * @return Ordered pair for sets and contact tracking.
     */
    static EntityPair MakeOrderedPair(Entity a, Entity b);

    /**
     * @brief True if the two AABBs overlap (call only for @ref GetCollidableEntities results).
     * @param registry Component source.
     * @param a First entity (Position + Collider).
     * @param b Second entity (Position + Collider).
     * @return Whether the two boxes overlap in world space.
     */
    static bool AreOverlapping(Registry& registry, Entity a, Entity b);

    /**
     * @brief True if the entity has an enabled collider and a position.
     */
    static bool IsCollidable(Registry& registry, Entity entity);

    /**
     * @brief Fills @p collidableEntities and @p movedEntities from @ref m_Entities.
     * @param registry ECS registry.
     * @param collidableEntities Output: entities with position and enabled collider.
     * @param movedEntities Output: collidable entities with @ref Position::moved set.
     */
    void GetCollidableEntities(Registry& registry, std::vector<Entity>& collidableEntities,
                               std::vector<Entity>& movedEntities) const;

    /**
     * @brief Runs onCollisionEnter callbacks on both entities when present.
     * @param registry ECS registry.
     * @param a First entity in the contact pair.
     * @param b Second entity in the contact pair.
     */
    static void EnterContact(Registry& registry, Entity a, Entity b);

    /**
     * @brief Runs onCollisionExit callbacks on both entities when present.
     * @param registry ECS registry.
     * @param a First entity in the contact pair.
     * @param b Second entity in the contact pair.
     */
    static void ExitContact(Registry& registry, Entity a, Entity b);

    /** @brief Entity pairs currently overlapping (used to detect enter vs stay vs exit). */
    std::set<EntityPair> m_ActiveContacts;
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_COLLISIONSYSTEM_HPP_
