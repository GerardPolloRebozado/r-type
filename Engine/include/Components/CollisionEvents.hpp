#ifndef ENGINE_COMPONENTS_COLLISIONEVENTS_HPP_
#define ENGINE_COMPONENTS_COLLISIONEVENTS_HPP_

#include "ECS/Entity.hpp"
#include <functional>
#include <vector>

namespace Engine {
/**
 * @struct CollisionEvents
 * @brief Per-entity callbacks for collision lifetime (Unity / Godot style).
 *
 * @ref CollisionSystem invokes these when overlap with another collider begins
 * or ends. Callbacks must not destroy entities synchronously; use
 * @ref Registry::QueueDestroyEntity instead.
 */
struct CollisionEvents {
    /** @brief Fired once when a new overlap with @p other begins. */
    std::vector<std::function<void(Entity self, Entity other)>> onCollisionEnter;

    /** @brief Fired once when an overlap with @p other ends. */
    std::vector<std::function<void(Entity self, Entity other)>> onCollisionExit;
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_COLLISIONEVENTS_HPP_
