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
 * @ref CollisionSystem invokes these when overlap with another collider begins,
 * persists, or ends. Callbacks must not destroy entities synchronously; use
 * @ref Registry::QueueDestroyEntity instead.
 */
struct CollisionEvents {
    using Callback = std::function<void(Entity self, Entity other)>;

    /** @brief Fired once when a new overlap begins. */
    std::vector<Callback> onCollisionEnter;

    /** @brief Fired each frame while overlapping */
    std::vector<Callback> onCollisionStay;

    /** @brief Fired once when an overlap ends. */
    std::vector<Callback> onCollisionExit;

    /** @brief Fired once when a trigger overlap enters*/
    std::vector<Callback> onTriggerEnter;

    /** @brief Fired each frame while trigger overlapping. */
    std::vector<Callback> onTriggerStay;

    /** @brief Fired once when a trigger overlap ends */
    std::vector<Callback> onTriggerExit;

    /** @brief Clears all registered callbacks. */
    void Clear() {
        onCollisionEnter.clear();
        onCollisionStay.clear();
        onCollisionExit.clear();
        onTriggerEnter.clear();
        onTriggerStay.clear();
        onTriggerExit.clear();
    }
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_COLLISIONEVENTS_HPP_