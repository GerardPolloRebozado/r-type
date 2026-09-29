#include "Editor/EditorLayer.hpp"
#include "Editor/EditorStyle.hpp"
#include "Editor/Panels/ConsolePanel.hpp"
#include "Editor/Panels/HierarchyPanel.hpp"
#include "Editor/Panels/InspectorPanel.hpp"
#include "Editor/Panels/SceneViewPanel.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <rlImGui.h>

namespace Editor {

EditorLayer::EditorLayer() {
    AddPanel(std::make_unique<HierarchyPanel>());
    AddPanel(std::make_unique<InspectorPanel>());
    AddPanel(std::make_unique<ConsolePanel>());
    AddPanel(std::make_unique<SceneViewPanel>());
}
EditorLayer::~EditorLayer() {
    Shutdown();
}

void EditorLayer::Init() {
    // Set a callback to load custom fonts instead of the default rlImGui font
    rlImGuiSetLoadFontsCallback([]() {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF("Editor/assets/fonts/Inter-Regular.ttf", 16.0f);
    });

    rlImGuiBeginInitImGui();

    // Apply styling
    EditorStyle::Apply();

    rlImGuiEndInitImGui();

    // Enable docking feature
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void EditorLayer::Shutdown() {
    m_Panels.clear();
    rlImGuiShutdown();
}

void EditorLayer::AddPanel(std::unique_ptr<IEditorPanel> panel) {
    m_Panels.push_back(std::move(panel));
}

void EditorLayer::OnUpdate() {
    rlImGuiBegin();

    BeginDockSpace();

    // Render all registered panels
    for (auto& panel : m_Panels) {
        panel->OnImGuiRender();
    }

    EndDockSpace();

    rlImGuiEnd();
}

void EditorLayer::BeginDockSpace() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("MainDockSpaceWindow", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

    if (m_FirstTimeLayout) {
        m_FirstTimeLayout = false;
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

        ImGuiID dock_main_id = dockspace_id;
        ImGuiID dock_left_id =
            ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
        ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f,
                                                            nullptr, &dock_main_id);
        ImGuiID dock_down_id =
            ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);

        ImGui::DockBuilderDockWindow("Hierarchy", dock_left_id);
        ImGui::DockBuilderDockWindow("Inspector", dock_right_id);
        ImGui::DockBuilderDockWindow("Console", dock_down_id);
        ImGui::DockBuilderDockWindow("Scene", dock_main_id);

        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
}

void EditorLayer::EndDockSpace() {
    ImGui::End(); // End MainDockSpaceWindow
}

} // namespace Editor
