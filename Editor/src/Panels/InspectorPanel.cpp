#include "Panels/InspectorPanel.hpp"
#include "Core/Application.hpp"
#include "Plugins/PositionPluginEditor.hpp"
#include <cstring>
#include <imgui.h>

#include "Panels/HierarchyPanel.hpp"

namespace Editor {
InspectorPanel::InspectorPanel() {
    m_PluginEditors.push_back(std::make_unique<PositionPluginEditor>());
}

void InspectorPanel::OnImGuiRender(Engine::Registry* registry, EditorConfig* config) {
    auto& selected = registry->GetResource<HierarchyPanelSelectedEntity>();
    if (ImGui::Begin(m_Name.c_str())) {
        if (selected.entity != -1 && registry) {
            ImGui::TextDisabled("Entity %d", static_cast<int>(selected.entity));
            ImGui::Separator();

            for (auto& editor : m_PluginEditors) {
                editor->OnImGuiRender(selected.entity, *registry);
            }
        } else {
            ImGui::TextDisabled("No entity selected");
        }
    }

    if (ImGui::BeginPopupContextWindow("InspectorContextMenu",
                                       ImGuiPopupFlags_MouseButtonRight |
                                           ImGuiPopupFlags_NoOpenOverItems)) {
        if (selected.entity != -1 && registry) {
            bool anyAvailable = false;
            for (auto& editor : m_PluginEditors) {
                if (!editor->HasComponent(selected.entity, *registry)) {
                    anyAvailable = true;
                    std::string menuItem =
                        std::string(editor->GetIcon()) + " Add " + editor->GetName();
                    if (ImGui::MenuItem(menuItem.c_str())) {
                        editor->AddComponent(selected.entity, *registry);
                    }
                }
            }
            if (!anyAvailable) {
                ImGui::TextDisabled("All plugins added");
            }
        } else {
            ImGui::TextDisabled("No entity selected");
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}
} // namespace Editor

namespace Editor {
const std::string& InspectorPanel::GetName() const {
    return m_Name;
}
}

namespace Editor {
void InspectorPanel::SetSelectedEntity(Engine::Entity* entity) {
    m_SelectedEntity = (entity ? *entity : -1);
}
}
