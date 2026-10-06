#include "Panels/HierarchyPanel.hpp"
#include "Components/Position.hpp"
#include "EditorConfig.hpp"
#include <imgui.h>

namespace Editor {
void HierarchyPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    auto& selected = registry.GetResource<HierarchyPanelSelectedEntity>();
    if (ImGui::Begin(m_Name.c_str())) {
        // Right-click context menu in empty space
        if (ImGui::BeginPopupContextWindow("HierarchyContextMenu",
                                           ImGuiPopupFlags_MouseButtonRight |
                                               ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("Create Empty Entity")) {
                const Engine::Entity newEntity = registry.CreateEntity();
                // registry.AddComponent<Engine::Position>(newEntity, Engine::Position(0, 0));
                selected.entity = newEntity;
                selected.index = static_cast<int>(newEntity); // Temporary simple selection map
            }
            ImGui::EndPopup();
        }

        if (registry.GetEntityCount() == 0) {
            ImGui::TextDisabled("Right click to create an entity...");
        } else {
            for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
                const auto entity = i;
                std::string label = config.GetEntityName(entity) + "##" + std::to_string(entity);
                if (ImGui::Selectable(label.c_str(), selected.index == static_cast<int>(i))) {
                    selected.index = static_cast<int>(i);
                    selected.entity = entity;
                }
            }
        }
    }
    ImGui::End();
}
} // namespace Editor

namespace Editor {
const std::string& HierarchyPanel::GetName() const {
    return m_Name;
}
} // namespace Editor
