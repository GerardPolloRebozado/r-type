#include "Panels/InspectorPanel.hpp"
#include "Components/Name.hpp"
#include "Core/Application.hpp"
#include "Plugins/PositionPluginEditor.hpp"
#include <cstring>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "Panels/HierarchyPanel.hpp"
#include "Plugins/ColliderPluginEditor.hpp"
#include "Plugins/RotationPluginEditor.hpp"
#include "Plugins/ShapePluginEditor.hpp"
#include "Plugins/SpritePluginEditor.hpp"
#include "Plugins/TextPluginEditor.hpp"

namespace Editor {
InspectorPanel::InspectorPanel() {
    m_PluginEditors.push_back(std::make_unique<PositionPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<TextPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<ShapePluginEditor>());
    m_PluginEditors.push_back(std::make_unique<RotationPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<SpritePluginEditor>());
    m_PluginEditors.push_back(std::make_unique<ColliderPluginEditor>());
}

void InspectorPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    auto& selected = registry.GetResource<HierarchyPanelSelectedEntity>();
    if (ImGui::Begin(m_Name.c_str())) {
        if (selected.entity != -1) {
            const bool hasName = registry.HasComponent<Engine::Name>(selected.entity);
            std::string name =
                hasName ? registry.GetComponent<Engine::Name>(selected.entity).value : "";
            const std::string hint = "Entity " + std::to_string(selected.entity);
            ImGui::PushID(static_cast<int>(selected.entity));
            if (ImGui::InputTextWithHint("Name", hint.c_str(), &name)) {
                if (name.empty()) {
                    registry.RemoveComponent<Engine::Name>(selected.entity);
                } else if (hasName) {
                    registry.GetComponent<Engine::Name>(selected.entity).value = name;
                } else {
                    registry.AddComponent<Engine::Name>(selected.entity,
                                                        Engine::Name::Create(name));
                }
            }
            ImGui::PopID();
            ImGui::TextDisabled("ID: %d", static_cast<int>(selected.entity));
            ImGui::Separator();

            for (auto& editor : m_PluginEditors) {
                editor->OnImGuiRender(selected.entity, registry);
            }
        } else {
            ImGui::TextDisabled("No entity selected");
        }
    }

    if (ImGui::BeginPopupContextWindow("InspectorContextMenu",
                                       ImGuiPopupFlags_MouseButtonRight |
                                           ImGuiPopupFlags_NoOpenOverItems)) {
        if (selected.entity != -1) {
            bool anyAvailable = false;
            for (auto& editor : m_PluginEditors) {
                if (!editor->HasComponent(selected.entity, registry)) {
                    anyAvailable = true;
                    std::string menuItem =
                        std::string(editor->GetIcon()) + " Add " + editor->GetName();
                    if (ImGui::MenuItem(menuItem.c_str())) {
                        editor->AddComponent(selected.entity, registry);
                    }
                }
            }
            if (!anyAvailable) {
                ImGui::TextDisabled("All plugins added");
            }
        } else {
            ImGui::TextDisabled("No entity selected");
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}
} // namespace Editor

namespace Editor {
const std::string& InspectorPanel::GetName() const {
    return m_Name;
}
} // namespace Editor

namespace Editor {
void InspectorPanel::SetSelectedEntity(Engine::Entity entity) {
    m_SelectedEntity = entity;
}
} // namespace Editor