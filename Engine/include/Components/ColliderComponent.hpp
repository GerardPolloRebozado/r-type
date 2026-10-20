#ifndef ENGINE_COMPONENTS_COLLIDER_HPP_
#define ENGINE_COMPONENTS_COLLIDER_HPP_

#include "CollisionLayers.hpp"
#include "ShapeComponent.hpp"

#include <cstdint>
#include <optional>
#include <raylib.h>

namespace Engine {
/**
 * @struct ColliderComponent
 * @brief 2D Axis-aligned bounding box collider integrated with Raylib Rectangle.
 *
 * Two colliders only interact when each one's @ref mask contains the other's
 * @ref layer, so either side can restrict what it touches (see @ref
 * CollisionLayers). With the defaults every collider touches every other, which
 * is the behaviour scenes saved before layers existed keep having.
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

    /** @brief Layer this collider belongs to, in [0, @ref CollisionLayers::kLayerCount). */
    int layer = CollisionLayers::kDefaultLayer;

    /**
     * @brief Bitmask of the layers this collider is willing to touch.
     *        Defaults to @ref CollisionLayers::kAllLayers.
     */
    std::uint32_t mask = CollisionLayers::kAllLayers;

    /**
     * @brief True when @p other's layer is in this collider's @ref mask.
     */
    bool Accepts(const ColliderComponent& other) const {
        return (mask & CollisionLayers::Bit(other.layer)) != 0u;
    }

    /**
     * @brief The pair filter used by @ref CollisionSystem: both sides must accept
     *        each other, so a single restrictive mask skips the whole pair
     *        (detection, resolution and enter/stay/exit events).
     */
    static bool CanCollide(const ColliderComponent& a, const ColliderComponent& b) {
        return a.Accepts(b) && b.Accepts(a);
    }

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
