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

    auto& shape = registry.GetComponent<Engine::ShapeComponent>(entity);

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
        int currentType = static_cast<int>(shape.type);
        const char* types[] = {"Rectangle", "Circle", "Polygon", "Line"};
        if (ImGui::Combo("Type", &currentType, types, 4)) {
            shape.type = static_cast<Engine::ShapeType>(currentType);
        }

        bool filled = shape.filled;
        if (ImGui::Checkbox("Filled", &filled)) {
            shape.filled = filled;
        }

        float col[4] = {shape.color.r / 255.0f, shape.color.g / 255.0f, shape.color.b / 255.0f,
                        shape.color.a / 255.0f};
        if (ImGui::ColorEdit4("Color", col)) {
            shape.color.r = static_cast<unsigned char>(col[0] * 255.0f);
            shape.color.g = static_cast<unsigned char>(col[1] * 255.0f);
            shape.color.b = static_cast<unsigned char>(col[2] * 255.0f);
            shape.color.a = static_cast<unsigned char>(col[3] * 255.0f);
        }

        switch (shape.type) {
            case Engine::ShapeType::RECTANGLE:
                ImGui::DragFloat("Width", &shape.width, 1.0f, 0.0f, 10000.0f);
                ImGui::DragFloat("Height", &shape.height, 1.0f, 0.0f, 10000.0f);
                break;
            case Engine::ShapeType::CIRCLE:
                ImGui::DragFloat("Radius", &shape.radius, 1.0f, 0.0f, 10000.0f);
                break;
            case Engine::ShapeType::POLYGON:
                ImGui::DragFloat("Radius", &shape.radius, 1.0f, 0.0f, 10000.0f);
                ImGui::DragInt("Sides", &shape.sides, 1.0f, 3, 32);
                if (!shape.filled) {
                    ImGui::DragFloat("Thickness", &shape.thickness, 0.1f, 0.1f, 100.0f);
                }
                break;
            case Engine::ShapeType::LINE:
                ImGui::DragFloat2("End Offset", &shape.endOffset.x, 1.0f);
                ImGui::DragFloat("Thickness", &shape.thickness, 0.1f, 0.1f, 100.0f);
                break;
        }
    }

    if (remove) {
        RemoveComponent(entity, registry);
        return false;
    }

    return true;
}

void ShapePluginEditor::Serialize(Engine::Entity entity, Engine::Registry& registry,
                                  nlohmann::json& outEntityJson) {
    if (registry.HasComponent<Engine::ShapeComponent>(entity)) {
        auto& shape = registry.GetComponent<Engine::ShapeComponent>(entity);
        outEntityJson["ShapeComponent"] = {
            {"type", static_cast<int>(shape.type)},
            {"width", shape.width},
            {"height", shape.height},
            {"radius", shape.radius},
            {"sides", shape.sides},
            {"endOffsetX", shape.endOffset.x},
            {"endOffsetY", shape.endOffset.y},
            {"thickness", shape.thickness},
            {"filled", shape.filled},
            {"color_r", shape.color.r},
            {"color_g", shape.color.g},
            {"color_b", shape.color.b},
            {"color_a", shape.color.a},
        };
    }
}

void ShapePluginEditor::Deserialize(Engine::Entity entity, Engine::Registry& registry,
                                    const nlohmann::json& inEntityJson) {
    if (inEntityJson.contains("ShapeComponent")) {
        const auto& json = inEntityJson["ShapeComponent"];
        Engine::ShapeComponent shape;
        shape.type = static_cast<Engine::ShapeType>(json.value("type", 0));
        shape.width = json.value("width", 50.0f);
        shape.height = json.value("height", 50.0f);
        shape.radius = json.value("radius", 25.0f);
        shape.sides = json.value("sides", 3);
        shape.endOffset.x = json.value("endOffsetX", 50.0f);
        shape.endOffset.y = json.value("endOffsetY", 50.0f);
        shape.thickness = json.value("thickness", 1.0f);
        shape.filled = json.value("filled", true);
        shape.color.r = json.value("color_r", 255);
        shape.color.g = json.value("color_g", 255);
        shape.color.b = json.value("color_b", 255);
        shape.color.a = json.value("color_a", 255);

        Engine::ShapePlugin::AttachToEntity(registry, entity, shape);
    }
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
