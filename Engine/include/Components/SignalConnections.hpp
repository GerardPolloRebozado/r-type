#ifndef ENGINE_COMPONENTS_SIGNALCONNECTIONS_HPP_
#define ENGINE_COMPONENTS_SIGNALCONNECTIONS_HPP_

#include "ECS/Entity.hpp"

#include <array>
#include <string>
#include <string_view>
#include <vector>

namespace Engine {

/**
 * @brief Collision/trigger signal names emitted by @ref CollisionSystem.
 *
 * These double as the Godot-style signal vocabulary for connections: an
 * emitter with a Collider fires the `onCollision*` signals on every contact
 * and additionally the `onTrigger*` ones when either party is a trigger.
 */
inline constexpr std::array<std::string_view, 6> kCollisionSignals = {
    "onCollisionEnter", "onCollisionStay", "onCollisionExit",
    "onTriggerEnter",   "onTriggerStay",   "onTriggerExit",
};

/**
 * @brief True when @p signal is a signal the engine can emit.
 */
inline bool IsKnownSignal(std::string_view signal) {
    for (const auto& known : kCollisionSignals) {
        if (known == signal) {
            return true;
        }
    }
    return false;
}

/**
 * @struct SignalConnection
 * @brief Godot-style connection: when @ref emitter fires @ref signal, call
 *        @ref method on @ref target's script table as `method(table, emitter,
 *        other)`.
 *
 * Stored as plain data (never Lua closures) so connections survive scene
 * save/load and script reloads: the method is re-resolved at dispatch time.
 */
struct SignalConnection {
    std::string signal;
    Entity target = static_cast<Entity>(-1);
    std::string method;
};

/**
 * @struct SignalConnections
 * @brief All outgoing connections of one emitter entity.
 */
struct SignalConnections {
    std::vector<SignalConnection> connections;
};

} // namespace Engine

#endif // ENGINE_COMPONENTS_SIGNALCONNECTIONS_HPP_
