#include "Panels/UiPalettePanel.hpp"
#include "EditorConfig.hpp"
#include "UI/UiCatalog.hpp"
#include <imgui.h>
#include <string>

namespace Editor {

bool UiPalettePanel::s_IsOpen = true;

UiPalettePanel::UiPalettePanel() : m_Name("UI Palette") {}

const std::string& UiPalettePanel::GetName() const {
    return m_Name;
}

void UiPalettePanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    int index = 0;
    std::string group;
    std::string payload;

    (void)registry;
    (void)config;
    if (!s_IsOpen) {
        return;
    }
    if (!ImGui::Begin(m_Name.c_str(), &s_IsOpen)) {
        ImGui::End();
        return;
    }
    ImGui::TextWrapped(
        "Drop a screen (Menu, Settings) to make a page, then drop widgets onto it in the hierarchy.");
    for (index = 0; index < Engine::UiTemplateCount(); ++index) {
        const Engine::UiTemplateInfo& info = Engine::UiTemplates()[index];
        if (group != info.group) {
            group = info.group;
            ImGui::SeparatorText(group.c_str());
        }
        ImGui::Selectable(info.label);
        if (ImGui::BeginDragDropSource()) {
            payload = info.id;
            ImGui::SetDragDropPayload("UI_TEMPLATE", payload.c_str(), payload.size() + 1);
            ImGui::TextUnformatted(info.label);
            ImGui::EndDragDropSource();
        }
    }
    ImGui::End();
}

} // namespace Editor
