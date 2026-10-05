#include "Panels/SceneViewPanel.hpp"
#include "Core/Application.hpp"
#include <imgui.h>

namespace Editor {

void SceneViewPanel::OnImGuiRender(Engine::Registry* registry, EditorConfig*) {
    if (ImGui::Begin(m_Name.c_str())) {
        ImGui::Text("Game viewport will be rendered here...");
        // TODO: Render a Framebuffer texture from Raylib, or something, idk yet
    }
    ImGui::End();
}

} // namespace Editor

namespace Editor {
const std::string& SceneViewPanel::GetName() const {
    return m_Name;
}
}
