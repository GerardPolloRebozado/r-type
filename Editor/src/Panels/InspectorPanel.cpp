#include "Editor/Panels/InspectorPanel.hpp"
#include <imgui.h>

namespace Editor {

void InspectorPanel::OnImGuiRender() {
    if (ImGui::Begin(m_Name.c_str())) {
        ImGui::Text("Entity properties go here...");
        // TODO: Show components for selected entity
    }
    ImGui::End();
}

} // namespace Editor
