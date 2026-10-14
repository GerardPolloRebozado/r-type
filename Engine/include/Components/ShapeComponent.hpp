#ifndef ENGINE_COMPONENTS_SHAPECOMPONENT_HPP_
#define ENGINE_COMPONENTS_SHAPECOMPONENT_HPP_

#include "Position.hpp"
#include "Rotation.hpp"
#include "raylib-cpp.hpp"
#include <cmath>
#include <numbers>
#include <vector>

/**
 * @file ShapeComponent.hpp
 * @brief Declares the screen-space shape component and its supporting types.
 */
namespace Engine {

/**
 * @enum ShapeType
 * @brief Supported procedural shape geometries.
 */
enum class ShapeType { RECTANGLE, CIRCLE, POLYGON, LINE };

/**
 * @struct ShapeComponent
 * @brief Plain data component describing a shape to render.
 */
struct ShapeComponent {
    /** @brief Geometry type selectable by this component. */
    ShapeType type = ShapeType::RECTANGLE;

    /** @brief Rectangle width. */
    float width = 50.0f;

    /** @brief Rectangle height. */
    float height = 50.0f;

    /** @brief Radius used when @ref type is Circle or Polygon. */
    float radius = 25.0f;

    /** @brief Number of sides used when @ref type is Polygon. */
    int sides = 3;

    /** @brief End offset used when @ref type is Line. */
    Vector2 endOffset = {50.0f, 50.0f};

    /** @brief Line thickness used when @ref type is Line. */
    float thickness = 1.0f;

    /** @brief Whether the shape is filled or outlined. */
    bool filled = true;

    /** @brief Render color. */
    mutable Color color = WHITE;

    ShapeComponent() = default;

    /**
     * @brief Computes the transformed polygon points representing this shape.
     * @param rot Rotation of the owning entity.
     * @param pos Position of the owning entity.
     * @return Polygon vertices in world/screen space.
     */
    std::vector<raylib::Vector2> GetShapeVectors(const Rotation& rot, const Position& pos) const;
};

} // namespace Engine

#endif // ENGINE_COMPONENTS_SHAPECOMPONENT_HPP_