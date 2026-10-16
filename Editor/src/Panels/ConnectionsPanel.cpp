#include "Panels/ConnectionsPanel.hpp"
#include "Plugins/SignalPlugin.hpp"
#include <imgui.h>

namespace Editor {

void ConnectionsPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    (void)config;

    if (!ImGui::Begin(m_Name.c_str())) {
        ImGui::End();
        return;
    }

    if (!registry.HasResource<HierarchyPanelSelectedEntity>()) {
        ImGui::TextDisabled("No entity selected.");
        ImGui::End();
        return;
    }
    const Engine::Entity selected = registry.GetResource<HierarchyPanelSelectedEntity>().entity;
    if (selected == static_cast<Engine::Entity>(-1) || !registry.IsEntityAlive(selected)) {
        ImGui::TextDisabled("Select an entity in the Hierarchy to manage its connections.");
        ImGui::End();
        return;
    }

    if (!m_Editor.HasComponent(selected, registry)) {
        ImGui::TextDisabled("Selected entity has no connections.");
        if (ImGui::Button("Add Connections component")) {
            m_Editor.AddComponent(selected, registry);
        }
        ImGui::End();
        return;
    }

    m_Editor.RenderTabs(selected, registry);
    ImGui::End();
}

const std::string& ConnectionsPanel::GetName() const {
    return m_Name;
}

} // namespace Editor
