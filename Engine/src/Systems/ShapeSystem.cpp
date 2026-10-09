#include "Systems/ShapeSystem.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "ECS/Registry.hpp"
#include "raylib.h"

namespace Engine {

void ShapeSystem::Render(Registry& registry) {
    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Entity entity{i};
        if (registry.HasComponent<Position>(entity) &&
            registry.HasComponent<ShapeComponent>(entity)) {
            auto& pos = registry.GetComponent<Position>(entity);
            auto& shape = registry.GetComponent<ShapeComponent>(entity);

            float rotation = 0.0f;
            if (registry.HasComponent<Rotation>(entity)) {
                rotation = registry.GetComponent<Rotation>(entity).angle;
            }

            switch (shape.type) {
                case ShapeType::RECTANGLE:
                    if (shape.filled) {
                        DrawRectangle(pos.x, pos.y, shape.width, shape.height, shape.color);
                    } else {
                        DrawRectangleLines(pos.x, pos.y, shape.width, shape.height, shape.color);
                    }
                    break;
                case ShapeType::CIRCLE:
                    if (shape.filled) {
                        DrawCircle(pos.x, pos.y, shape.radius, shape.color);
                    } else {
                        DrawCircleLines(pos.x, pos.y, shape.radius, shape.color);
                    }
                    break;
                case ShapeType::POLYGON:
                    if (shape.filled) {
                        DrawPoly({pos.x, pos.y}, shape.sides, shape.radius, rotation, shape.color);
                    } else {
                        DrawPolyLinesEx({pos.x, pos.y}, shape.sides, shape.radius, rotation,
                                        shape.thickness, shape.color);
                    }
                    break;
                case ShapeType::LINE:
                    DrawLineEx({pos.x, pos.y},
                               {pos.x + shape.endOffset.x, pos.y + shape.endOffset.y},
                               shape.thickness, shape.color);
                    break;
            }
        }
    }
}

} // namespace Engine
