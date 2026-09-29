#include "Panels/HierarchyPanel.hpp"
#include <imgui.h>

namespace Editor {

void HierarchyPanel::OnImGuiRender() {
    if (ImGui::Begin(m_Name.c_str())) {
        ImGui::Text("Entity list goes here...");
        // TODO: entity listing
    }
    ImGui::End();
}

} // namespace Editor
