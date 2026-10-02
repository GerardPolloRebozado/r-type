#include "Panels/InspectorPanel.hpp"
#include "Plugins/PositionPluginEditor.hpp"
#include <cstring>
#include <imgui.h>

namespace Editor {

InspectorPanel::InspectorPanel() {
    m_PluginEditors.push_back(std::make_unique<PositionPluginEditor>());
}

void InspectorPanel::OnImGuiRender(Engine::Registry* registry) {
    if (ImGui::Begin(m_Name.c_str())) {
        if (m_SelectedEntity != -1 && registry) {
            ImGui::TextDisabled("Entity %d", (int)m_SelectedEntity);
            ImGui::Separator();

            for (auto& editor : m_PluginEditors) {
                editor->OnImGuiRender(m_SelectedEntity, *registry);
            }
        } else {
            ImGui::TextDisabled("No entity selected");
        }
    }
    ImGui::End();
}

} // namespace Editor
