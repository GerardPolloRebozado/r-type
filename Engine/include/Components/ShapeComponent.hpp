#ifndef ENGINE_COMPONENTS_SHAPECOMPONENT_HPP_
#define ENGINE_COMPONENTS_SHAPECOMPONENT_HPP_

#include "raylib-cpp.hpp"

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
    Color color = WHITE;

    ShapeComponent() = default;
};

} // namespace Engine

#endif // ENGINE_COMPONENTS_SHAPECOMPONENT_HPP_
