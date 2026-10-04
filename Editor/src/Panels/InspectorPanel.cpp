#include "Panels/InspectorPanel.hpp"
#include "Core/Application.hpp"
#include "Plugins/TransformPluginEditor.hpp"
#include <imgui.h>

#include "Panels/HierarchyPanel.hpp"

namespace Editor {
    InspectorPanel::InspectorPanel() {
        m_PluginEditors.push_back(std::make_unique<TransformPluginEditor>());
    }

    void InspectorPanel::OnImGuiRender(Engine::Registry *registry, EditorConfig *config) {
        auto &selected = registry->GetResource<HierarchyPanelSelectedEntity>();
        if (ImGui::Begin(m_Name.c_str())) {
            if (selected.entity != -1 && registry) {
                ImGui::TextDisabled("Entity %d", static_cast<int>(selected.entity));
                ImGui::Separator();

                for (auto &editor: m_PluginEditors) {
                    editor->OnImGuiRender(selected.entity, *registry);
                }
            } else {
                ImGui::TextDisabled("No entity selected");
            }
        }

        if (ImGui::BeginPopupContextWindow("InsPectorContextMenu",
                                           ImGuiPopupFlags_MouseButtonRight |
                                           ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("Add component")) {
                auto &selectedEntity = registry->GetResource<HierarchyPanelSelectedEntity>();
                registry->AddComponent<Engine::Position>(selectedEntity.entity, Engine::Position(0, 0));
            }
            ImGui::EndPopup();
        }
        ImGui::End();
    }
} // namespace Editor
