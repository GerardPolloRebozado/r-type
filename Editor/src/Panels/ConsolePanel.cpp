#include "Panels/ConsolePanel.hpp"
#include <imgui.h>

namespace Editor {

void ConsolePanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    if (ImGui::Begin(m_Name.c_str())) {
        ImGui::Text("Engine logs go here...");
        // TODO: engine logger
    }
    ImGui::End();
}

} // namespace Editor

namespace Editor {
const std::string& ConsolePanel::GetName() const {
    return m_Name;
}
} // namespace Editor
