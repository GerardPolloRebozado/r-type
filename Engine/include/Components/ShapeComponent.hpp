#ifndef ENGINE_COMPONENTS_SHAPECOMPONENT_HPP_
#define ENGINE_COMPONENTS_SHAPECOMPONENT_HPP_

#include "Position.hpp"
#include "Rotation.hpp"
#include "raylib-cpp.hpp"
#include <cmath>
#include <numbers>
#include <vector>

namespace Engine {

enum class ShapeType { RECTANGLE, CIRCLE, POLYGON, LINE };

struct ShapeComponent {
    ShapeType type = ShapeType::RECTANGLE;

    // Rectangle
    float width = 50.0f;
    float height = 50.0f;

    // Circle & Polygon
    float radius = 25.0f;
    int sides = 3;

    // Line
    Vector2 endOffset = {50.0f, 50.0f};
    float thickness = 1.0f;

    // Common
    bool filled = true;
    mutable Color color = WHITE;

    std::vector<raylib::Vector2> GetShapeVectors(const Rotation& rot, const Position& pos) const;
};

} // namespace Engine

#endif // ENGINE_COMPONENTS_SHAPECOMPONENT_HPP_