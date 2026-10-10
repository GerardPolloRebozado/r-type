#ifndef ENGINE_COMPONENTS_COLLIDER_HPP_
#define ENGINE_COMPONENTS_COLLIDER_HPP_

#include "ShapeComponent.hpp"

#include <optional>
#include <raylib.h>

namespace Engine {
/**
 * @struct Collider
 * @brief 2D Axis-aligned bounding box collider integrated with Raylib Rectangle.
 */
struct Collider {

    /**
     * @brief Optional as the entity already has a shape, if a shape is defined in the collider, the
     * one in the collider takes preference to designate the area
     */
    std::optional<ShapeComponent> shape;

    /** @brief When false, skipped by CollisionSystem. */
    bool enabled;

    /** @brief When true, reports trigger events without solid blocking. */
    bool isTrigger;

    /**
     * @brief Constructs a collider with explicit dimensions and offsets.
     */
    Collider(const bool enabled = true, const bool isTrigger = false)
        : enabled(enabled), isTrigger(isTrigger) {}

    /**
     * @brief Constructs a collider directly from a Raylib Rectangle (x = offsetX, y = offsetY).
     */
    explicit Collider(const std::optional<ShapeComponent>& shape, const bool enabled = true,
                      const bool isTrigger = false)
        : shape(shape), enabled(enabled), isTrigger(isTrigger) {}
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_COLLIDER_HPP_