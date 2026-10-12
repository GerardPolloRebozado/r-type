#include "Plugins/RotationPluginEditor.hpp"
#include "Components/Rotation.hpp"
#include "Plugins/RotationPlugin.hpp"
#include <cmath>
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace Editor {

const char* RotationPluginEditor::GetName() const {
    return "Rotation";
}

const char* RotationPluginEditor::GetIcon() const {
    return ICON_FA_ROTATE;
}

bool RotationPluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::Rotation>(entity)) {
        return false;
    }

    auto& rot = registry.GetComponent<Engine::Rotation>(entity);

    const std::string header = std::string(GetIcon()) + " " + GetName();
    const bool isOpen = ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    if (isOpen) {
        // Simple dial/gizmo for rotation
        ImGui::DragFloat("Angle (deg)", &rot.angle, 1.0f, 0.0f, 360.0f);

        // Draw a small knob
        ImVec2 p = ImGui::GetCursorScreenPos();
        float radius = 20.0f;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImGui::InvisibleButton("rotation_knob", ImVec2(radius * 2, radius * 2));
        bool is_active = ImGui::IsItemActive();
        bool is_hovered = ImGui::IsItemHovered();

        ImVec2 center = ImVec2(p.x + radius, p.y + radius);
        float angle_rad = rot.angle * (3.1415926535f / 180.0f);

        if (is_active) {
            ImVec2 mouse_pos = ImGui::GetIO().MousePos;
            float dy = mouse_pos.y - center.y;
            float dx = mouse_pos.x - center.x;
            float new_angle_rad = std::atan2(dy, dx);
            float new_angle_deg = new_angle_rad * (180.0f / 3.1415926535f);
            if (new_angle_deg < 0) {
                new_angle_deg += 360.0f;
            }
            rot.angle = new_angle_deg;
            angle_rad = new_angle_rad;
        }

        ImU32 circle_col =
            is_active ? IM_COL32(200, 200, 200, 255)
                      : (is_hovered ? IM_COL32(150, 150, 150, 255) : IM_COL32(100, 100, 100, 255));
        draw_list->AddCircleFilled(center, radius, IM_COL32(50, 50, 50, 255), 32);
        draw_list->AddCircle(center, radius, circle_col, 32, 2.0f);

        ImVec2 line_end = ImVec2(center.x + std::cos(angle_rad) * radius,
                                 center.y + std::sin(angle_rad) * radius);
        draw_list->AddLine(center, line_end, IM_COL32(255, 100, 100, 255), 2.0f);
    }

    return true;
}

bool RotationPluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::Rotation>(entity);
}

void RotationPluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    Engine::RotationPlugin::AttachToEntity(registry, entity, 0.0f);
}

void RotationPluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.RemoveComponent<Engine::Rotation>(entity);
}

} // namespace Editor
