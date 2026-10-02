#ifndef ENGINE_COMPONENTS_COLLIDER_HPP_
#define ENGINE_COMPONENTS_COLLIDER_HPP_

namespace Engine {
/**
 * @struct Collider
 * @brief Axis-aligned bounding box (AABB) data attached to an entity.
 *
 * World-space position comes from the @ref Position on the same entity.
 * The box is centered on (Position.x + offsetX, Position.y + offsetY) with
 * the given half-extents or full size (see width/height below).
 */
struct Collider {
    /** @brief Total width of the box in world units (not half-extent). */
    float width;

    /** @brief Total height of the box in world units (not half-extent). */
    float height;

    /** @brief Horizontal offset from Position.x to the box center. */
    float offsetX;

    /** @brief Vertical offset from Position.y to the box center. */
    float offsetY;

    /**
     * @brief When false, this collider is skipped by @ref CollisionSystem.
     */
    bool enabled;

    /**
     * @brief When true, overlaps are reported but reserved for future
     *        physics response (no blocking movement in the current milestone).
     */
    bool isTrigger;

    /**
     * @brief Constructs a collider with explicit size and defaults.
     * @param width Box width in world units.
     * @param height Box height in world units.
     * @param offsetX Offset from entity position to box center on X.
     * @param offsetY Offset from entity position to box center on Y.
     * @param enabled Whether collision queries include this collider.
     * @param isTrigger Trigger vs solid semantics for future physics.
     */
    Collider(float width = 1.0f, float height = 1.0f, float offsetX = 0.0f, float offsetY = 0.0f,
             bool enabled = true, bool isTrigger = false)
        : width(width), height(height), offsetX(offsetX), offsetY(offsetY), enabled(enabled),
          isTrigger(isTrigger) {}
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_COLLIDER_HPP_
