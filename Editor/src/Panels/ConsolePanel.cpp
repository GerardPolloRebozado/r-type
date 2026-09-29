#include "Editor/Panels/ConsolePanel.hpp"
#include <imgui.h>

namespace Editor {

void ConsolePanel::OnImGuiRender() {
    if (ImGui::Begin(m_Name.c_str())) {
        ImGui::Text("Engine logs go here...");
        // TODO: engine logger
    }
    ImGui::End();
}

} // namespace Editor
