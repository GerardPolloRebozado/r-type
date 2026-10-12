#ifndef ENGINE_SYSTEMS_COLLISIONSYSTEM_HPP_
#define ENGINE_SYSTEMS_COLLISIONSYSTEM_HPP_

#include "Components/ColliderComponent.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "ECS/System.hpp"
#include <set>
#include <utility>
#include <vector>

namespace Engine {
/**
 * @class CollisionSystem
 * @brief Detects overlaps between colliders using Raylib and fires enter/stay/exit events.
 */
class CollisionSystem : public System {
  public:
    using EntityPair = std::pair<Entity, Entity>;

    ~CollisionSystem() noexcept override = default;

    /**
     * @brief Runs overlap detection and updates contact state for this frame.
     * @param registry ECS registry with positions and colliders.
     * @param dt Delta time.
     */
    void Update(Registry& registry, float dt) override;

    /**
     * @brief Returns a const reference to all active contact pairs this frame.
     */
    [[nodiscard]] const std::set<EntityPair>& GetActiveContacts() const noexcept {
        return m_ActiveContacts;
    }

  private:
    std::set<EntityPair> m_ActiveContacts;

    static EntityPair MakeOrderedPair(Entity a, Entity b) noexcept;

    static bool IsCollidable(Registry& registry, Entity entity);

    void GetCollidableEntities(Registry& registry, std::vector<Entity>& collidableEntities) const;

    static void EnterContact(Registry& registry, Entity a, Entity b, bool isTrigger);

    static void StayContact(Registry& registry, Entity a, Entity b, bool isTrigger);

    static void ExitContact(Registry& registry, Entity a, Entity b, bool isTrigger);

    /**
     * @brief This function given a shape, rotation and position returns a pair of max and min, this
     * pair is the result of the projection of such polygon https://dyn4j.org/2010/01/sat/#sat-proj
     * @param axis axis to project
     * @param vertices vertices of the shape to project
     * @return
     */
    static std::pair<float, float> ProjectOnAxis(const std::vector<raylib::Vector2>& vertices,
                                                 Vector2 axis);

    static bool CheckPolygonCollision(const std::vector<raylib::Vector2>& polyA,
                                      const std::vector<raylib::Vector2>& polyB);
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_COLLISIONSYSTEM_HPP_