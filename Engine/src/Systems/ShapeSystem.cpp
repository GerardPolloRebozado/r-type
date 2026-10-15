#include "Systems/ShapeSystem.hpp"
#include "Components/Opacity.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ScreenSpace.hpp"
#include "Components/ShapeComponent.hpp"
#include "ECS/Registry.hpp"
#include "Plugins/OpacityPlugin.hpp"
#include <raylib.h>
#include <rlgl.h>

namespace Engine {

bool ShapeSystem::CanDraw(Registry& registry, Entity entity, bool screenSpace) {
    return registry.HasComponent<ShapeComponent>(entity) &&
           registry.HasComponent<ScreenSpace>(entity) == screenSpace;
}

void ShapeSystem::DrawEntity(Registry& registry, Entity entity) {
    auto& pos = registry.GetComponent<Position>(entity);
    auto& shape = registry.GetComponent<ShapeComponent>(entity);

    // Opacity multiplies the shape's own alpha; a copy avoids
    // mutating the stored component.
    // children without their own Opacity still
    // inherit faded ancestors (1.0 when fully opaque = no change).
    Color drawColor = shape.color;
    drawColor.a = static_cast<unsigned char>(static_cast<float>(drawColor.a) *
                                             OpacityPlugin::GetEffectiveAlpha(registry, entity));

    float rotation = 0.0f;
    if (registry.HasComponent<Rotation>(entity)) {
        rotation = registry.GetComponent<Rotation>(entity).angle;
    }

    bool useRlgl =
        (rotation != 0.0f && (shape.type == ShapeType::RECTANGLE || shape.type == ShapeType::LINE));
    if (useRlgl) {
        rlPushMatrix();
        if (shape.type == ShapeType::RECTANGLE) {
            // Rotate around the center of the rectangle
            rlTranslatef(pos.x + shape.width / 2.0f, pos.y + shape.height / 2.0f, 0.0f);
            rlRotatef(rotation, 0.0f, 0.0f, 1.0f);
            rlTranslatef(-(pos.x + shape.width / 2.0f), -(pos.y + shape.height / 2.0f), 0.0f);
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
                DrawRectangle(pos.x, pos.y, shape.width, shape.height, drawColor);
            } else {
                DrawRectangleLinesEx({pos.x, pos.y, shape.width, shape.height}, shape.thickness,
                                     drawColor);
            }
            break;
        case ShapeType::CIRCLE:
            if (shape.filled) {
                DrawCircle(pos.x, pos.y, shape.radius, drawColor);
            } else {
                DrawCircleLines(pos.x, pos.y, shape.radius, drawColor);
            }
            break;
        case ShapeType::POLYGON:
            if (shape.filled) {
                DrawPoly({pos.x, pos.y}, shape.sides, shape.radius, rotation, drawColor);
            } else {
                DrawPolyLinesEx({pos.x, pos.y}, shape.sides, shape.radius, rotation,
                                shape.thickness, drawColor);
            }
            break;
        case ShapeType::LINE:
            DrawLineEx({pos.x, pos.y}, {pos.x + shape.endOffset.x, pos.y + shape.endOffset.y},
                       shape.thickness, drawColor);
            break;
    }

    if (useRlgl) {
        rlPopMatrix();
    }
}

} // namespace Engine