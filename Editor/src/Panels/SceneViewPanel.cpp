#include "Editor/Panels/SceneViewPanel.hpp"
#include <imgui.h>

namespace Editor {

void SceneViewPanel::OnImGuiRender() {
    if (ImGui::Begin(m_Name.c_str())) {
        ImGui::Text("Game viewport will be rendered here...");
        // TODO: Render a Framebuffer texture from Raylib, or something, idk yet
    }
    ImGui::End();
}

} // namespace Editor
