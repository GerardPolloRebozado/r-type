#include "Plugins/ShapePluginEditor.hpp"
#include "Components/ShapeComponent.hpp"
#include "Plugins/ShapePlugin.hpp"
// clang-format off
#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"
// clang-format on
#include <extras/IconsFontAwesome6.h>

namespace Editor {

const char* ShapePluginEditor::GetName() const {
    return "Shape";
}

const char* ShapePluginEditor::GetIcon() const {
    return ICON_FA_SHAPES;
}

bool ShapePluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::ShapeComponent>(entity)) {
        return false;
    }

    auto& shape = registry.RequireComponent<Engine::ShapeComponent>(entity);

    const std::string header = std::string(GetIcon()) + " " + GetName();
    const bool isOpen = ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    bool remove = false;
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Remove Component")) {
            remove = true;
        }
        ImGui::EndPopup();
    }

    if (isOpen) {
        DrawShapeControls(shape, true);
    }

    if (remove) {
        RemoveComponent(entity, registry);
        return false;
    }

    return true;
}

bool ShapePluginEditor::DrawShapeControls(Engine::ShapeComponent& shape, bool showVisualProps) {
    bool modified = false;
    std::string id = "###";
    if (showVisualProps) {
        id += '1';
    }

    int currentType = static_cast<int>(shape.type);
    const char* types[] = {"Rectangle", "Circle", "Polygon", "Line"};
    if (ImGui::Combo(("Type" + id + "type").c_str(), &currentType, types, 4)) {
        shape.type = static_cast<Engine::ShapeType>(currentType);
        modified = true;
    }

    if (showVisualProps) {
        bool filled = shape.filled;
        if (ImGui::Checkbox("Filled", &filled)) {
            shape.filled = filled;
            modified = true;
        }

        float col[4] = {shape.color.r / 255.0f, shape.color.g / 255.0f, shape.color.b / 255.0f,
                        shape.color.a / 255.0f};
        if (ImGui::ColorEdit4("Color", col)) {
            shape.color.r = static_cast<unsigned char>(col[0] * 255.0f);
            shape.color.g = static_cast<unsigned char>(col[1] * 255.0f);
            shape.color.b = static_cast<unsigned char>(col[2] * 255.0f);
            shape.color.a = static_cast<unsigned char>(col[3] * 255.0f);
            modified = true;
        }
    }

    switch (shape.type) {
        case Engine::ShapeType::RECTANGLE:
            if (ImGui::DragFloat(("Width" + id + "Width").c_str(), &shape.width, 1.0f, 0.0f,
                                 10000.0f)) {
                modified = true;
            }
            if (ImGui::DragFloat(("Height" + id + "Height").c_str(), &shape.height, 1.0f, 0.0f,
                                 10000.0f)) {
                modified = true;
            }
            break;
        case Engine::ShapeType::CIRCLE:
            if (ImGui::DragFloat(("Radius" + id + "Radius").c_str(), &shape.radius, 1.0f, 0.0f,
                                 10000.0f)) {
                modified = true;
            }
            break;
        case Engine::ShapeType::POLYGON:
            if (ImGui::DragFloat(("Radius" + id + "Radius").c_str(), &shape.radius, 1.0f, 0.0f,
                                 10000.0f)) {
                modified = true;
            }
            if (ImGui::DragInt(("Sides" + id + "Sides").c_str(), &shape.sides, 1.0f, 3, 32)) {
                modified = true;
            }
            if (!shape.filled || !showVisualProps) {
                if (ImGui::DragFloat(("Thickness" + id + "Thickness").c_str(), &shape.thickness,
                                     0.1f, 0.1f, 100.0f)) {
                    modified = true;
                }
            }
            break;
        case Engine::ShapeType::LINE:
            if (ImGui::DragFloat2(("End Offset" + id + "End Offset").c_str(), &shape.endOffset.x,
                                  1.0f)) {
                modified = true;
            }
            if (ImGui::DragFloat(("Thickness" + id + "Thickness").c_str(), &shape.thickness, 0.1f,
                                 0.1f, 100.0f)) {
                modified = true;
            }
            break;
    }

    return modified;
}

bool ShapePluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::ShapeComponent>(entity);
}

void ShapePluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    Engine::ShapeComponent defaultShape;
    Engine::ShapePlugin::AttachToEntity(registry, entity, defaultShape);
}

void ShapePluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.RemoveComponent<Engine::ShapeComponent>(entity);
}

} // namespace Editor