#ifndef ENGINE_COMPONENTS_COLLIDER_HPP_
#define ENGINE_COMPONENTS_COLLIDER_HPP_

#include "ShapeComponent.hpp"

#include <optional>
#include <raylib.h>

namespace Engine {
/**
 * @struct ColliderComponent
 * @brief 2D Axis-aligned bounding box collider integrated with Raylib Rectangle.
 */
struct ColliderComponent {

    /**
     * @brief Optional as the entity already has a shape, if a shape is defined in the collider, the
     * one in the collider takes preference to designate the area
     */
    std::optional<ShapeComponent> shape;

    /** @brief When false, skipped by CollisionSystem. */
    bool enabled = true;

    /** @brief When true, reports trigger events without solid blocking. */
    bool isTrigger = false;

    /** @brief When true, this collider does not move during resolution (acts like a wall/floor). */
    bool isStatic = false;

    /**
     * @brief Creates a collider with explicit dimensions and offsets.
     */
    static ColliderComponent Create(const bool enabled = true, const bool isTrigger = false) {
        ColliderComponent collider;
        collider.enabled = enabled;
        collider.isTrigger = isTrigger;
        return collider;
    }

    /**
     * @brief Creates a collider directly from a Raylib Rectangle (x = offsetX, y = offsetY).
     */
    static ColliderComponent Create(const std::optional<ShapeComponent>& shape,
                                    const bool enabled = true, const bool isTrigger = false) {
        ColliderComponent collider;
        collider.shape = shape;
        collider.enabled = enabled;
        collider.isTrigger = isTrigger;
        return collider;
    }
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_COLLIDER_HPP_