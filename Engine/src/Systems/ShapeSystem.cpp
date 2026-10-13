#include "Systems/ShapeSystem.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ScreenSpace.hpp"
#include "Components/ShapeComponent.hpp"
#include "ECS/Registry.hpp"
#include <raylib.h>
#include <rlgl.h>

namespace Engine {

void ShapeSystem::Render(Registry& registry) {
    DrawShapes(registry, false);
}

void ShapeSystem::RenderScreen(Registry& registry) {
    DrawShapes(registry, true);
}

void ShapeSystem::DrawShapes(Registry& registry, bool screenSpace) {
    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Entity entity{i};
        if (registry.HasComponent<Position>(entity) &&
            registry.HasComponent<ShapeComponent>(entity) &&
            registry.HasComponent<ScreenSpace>(entity) == screenSpace) {
            auto& pos = registry.GetComponent<Position>(entity);
            auto& shape = registry.GetComponent<ShapeComponent>(entity);

            float rotation = 0.0f;
            if (registry.HasComponent<Rotation>(entity)) {
                rotation = registry.GetComponent<Rotation>(entity).angle;
            }

            bool useRlgl = (rotation != 0.0f &&
                            (shape.type == ShapeType::RECTANGLE || shape.type == ShapeType::LINE));
            if (useRlgl) {
                rlPushMatrix();
                if (shape.type == ShapeType::RECTANGLE) {
                    // Rotate around the center of the rectangle
                    rlTranslatef(pos.x + shape.width / 2.0f, pos.y + shape.height / 2.0f, 0.0f);
                    rlRotatef(rotation, 0.0f, 0.0f, 1.0f);
                    rlTranslatef(-(pos.x + shape.width / 2.0f), -(pos.y + shape.height / 2.0f),
                                 0.0f);
                } else if (shape.type == ShapeType::LINE) {
                    // Rotate around the start point of the line
                    rlTranslatef(pos.x, pos.y, 0.0f);
                    rlRotatef(rotation, 0.0f, 0.0f, 1.0f);
                    rlTranslatef(-pos.x, -pos.y, 0.0f);
                }
            }

            switch (shape.type) {
                case ShapeType::RECTANGLE:
                    if (shape.filled) {
                        DrawRectangle(pos.x, pos.y, shape.width, shape.height, shape.color);
                    } else {
                        DrawRectangleLinesEx({pos.x, pos.y, shape.width, shape.height},
                                             shape.thickness, shape.color);
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

            if (useRlgl) {
                rlPopMatrix();
            }
        }
    }
}

} // namespace Engine
