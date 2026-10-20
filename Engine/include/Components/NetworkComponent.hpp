#ifndef ENGINE_COMPONENTS_NETWORKCOMPONENT_HPP
#define ENGINE_COMPONENTS_NETWORKCOMPONENT_HPP

#include <cstdint>
#include <string>

namespace Engine {

/**
 * @struct NetworkComponent
 * @brief Declarative networking metadata for a replicated entity.
 *
 * Attach this to an entity (usually on a prefab) to make it networked. The
 * replication system reads these fields to decide who simulates the entity and
 * which components sync — gameplay scripts never hand-roll spawn/broadcast.
 *
 * Authority models:
 *   - Server: the server simulates and broadcasts (enemies, physics bodies).
 *   - Client: the owning client sends input; the server validates and
 *             simulates. Used for per-player entities.
 *   - Shared: not synced. Every machine builds it from the same scene
 *             (level geometry, walls).
 */
struct NetworkComponent {
    /** Network id, assigned on spawn by the replication system. 0 = unassigned. */
    std::uint64_t networkId = 0;
    // Using string for authority to simplify serialization/deserialization.
    // Valid values: "Server", "Client", "Shared"
    std::string authority = "Server";
    /** Sync Position + Rotation each tick. */
    bool syncTransform = true;
    /** Sync SpriteComponent (path, offset, scale, flip). */
    bool syncSprite = false;
    /** When set, spawn this prefab instead of a bare entity on connect/spawn. */
    std::string prefabPath;
};

} // namespace Engine

#endif // ENGINE_COMPONENTS_NETWORKCOMPONENT_HPP
