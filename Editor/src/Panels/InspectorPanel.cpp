#include "Panels/InspectorPanel.hpp"
#include "TransformEditor.hpp"
#include <cstring>
#include <imgui.h>

namespace Editor {

InspectorPanel::InspectorPanel() {
    m_ComponentEditors.push_back(std::make_unique<TransformEditor>());
}

void InspectorPanel::OnImGuiRender(Engine::Registry* registry) {
    if (ImGui::Begin(m_Name.c_str())) {
        if (m_SelectedEntity != -1 && registry) {
            ImGui::TextDisabled("Entity %d", (int)m_SelectedEntity);
            ImGui::Separator();

            for (auto& editor : m_ComponentEditors) {
                editor->OnImGuiRender(m_SelectedEntity, *registry);
            }
        } else {
            ImGui::TextDisabled("No entity selected");
        }
    }
    ImGui::End();
}

} // namespace Editor
