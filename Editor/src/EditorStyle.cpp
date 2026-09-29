#include "Editor/EditorStyle.hpp"
#include <imgui.h>

namespace Editor {

void EditorStyle::Apply() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Unity Dark Theme Palette with Blueish Highlights
    colors[ImGuiCol_Text] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);

    // Blueish highlights
    colors[ImGuiCol_CheckMark] = ImVec4(0.24f, 0.48f, 0.73f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.48f, 0.73f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.28f, 0.56f, 0.85f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.48f, 0.73f, 0.80f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.48f, 0.73f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.24f, 0.48f, 0.73f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.56f, 0.85f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.32f, 0.64f, 0.97f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.24f, 0.48f, 0.73f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.28f, 0.56f, 0.85f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.24f, 0.48f, 0.73f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.28f, 0.56f, 0.85f, 1.00f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.24f, 0.48f, 0.73f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.24f, 0.48f, 0.73f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);

    colors[ImGuiCol_DockingPreview] = ImVec4(0.24f, 0.48f, 0.73f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 2.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 2.0f;
    style.TabRounding = 0.0f;
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.WindowPadding = ImVec2(4.0f, 4.0f);
}

} // namespace Editor
