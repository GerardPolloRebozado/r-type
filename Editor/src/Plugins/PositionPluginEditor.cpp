#include "Plugins/PositionPluginEditor.hpp"
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <string>

namespace Editor {

const char* PositionPluginEditor::GetName() const {
    return "Position";
}

const char* PositionPluginEditor::GetIcon() const {
    return ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT;
}

bool PositionPluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::Position>(entity)) {
        return false;
    }

    auto& position = registry.GetComponent<Engine::Position>(entity);

    std::string header = std::string(GetIcon()) + " " + GetName();
    bool isOpen = ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    if (isOpen) {
        float pos[2] = {position.x, position.y};
        if (ImGui::DragFloat2("Position", pos, 0.1f)) {
            position.x = pos[0];
            position.y = pos[1];
        }
        ImGui::DragInt("Z Index", &position.z);
    }

    return true;
}

bool PositionPluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::Position>(entity);
}

void PositionPluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.AddComponent<Engine::Position>(entity, Engine::Position(0, 0, 0));
}

void PositionPluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.RemoveComponent<Engine::Position>(entity);
}

} // namespace Editor
