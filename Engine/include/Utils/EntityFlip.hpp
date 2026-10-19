#ifndef ENGINE_UTILS_ENTITYFLIP_HPP_
#define ENGINE_UTILS_ENTITYFLIP_HPP_

#include "ECS/Entity.hpp"

namespace Engine {
class Registry;

/**
 * @brief Mirrors an entity rig horizontally, about its own centre.
 *
 * The entity's own sprite flip is toggled, so the art faces the other way, and
 * every descendant's footprint is reflected about the same vertical line, so
 * attachments - a front sensor, a weapon mount - swap sides while keeping their
 * distance from their parent. Reflection uses footprints rather than anchors
 * because positions are top-left corners in this engine: mirroring the anchor of
 * a box that grows rightwards would leave it on the wrong side of its parent.
 *
 * This is the implementation behind the `Component.flipX` Lua binding. It is a
 * toggle, so call it once per direction change.
 *
 * @param registry ECS registry holding the entity and its subtree.
 * @param entity Root of the rig to mirror.
 * @return false when @p entity is not alive or has no Position; the registry is
 *         left untouched in that case.
 */
bool FlipEntityX(Registry& registry, Entity entity);

} // namespace Engine

#endif // ENGINE_UTILS_ENTITYFLIP_HPP_
