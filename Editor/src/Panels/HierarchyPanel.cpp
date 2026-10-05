#include "Panels/HierarchyPanel.hpp"
#include "Components/Position.hpp"
#include <imgui.h>

namespace Editor {
    void HierarchyPanel::OnImGuiRender(Engine::Registry *registry, EditorConfig *) {
        auto &selected = registry->GetResource<HierarchyPanelSelectedEntity>();
        if (ImGui::Begin(m_Name.c_str())) {
            if (!registry) {
                ImGui::TextDisabled("No Engine Registry linked!");
            } else {
                // Right-click context menu in empty space
                if (ImGui::BeginPopupContextWindow("HierarchyContextMenu",
                                                   ImGuiPopupFlags_MouseButtonRight |
                                                   ImGuiPopupFlags_NoOpenOverItems)) {
                    if (ImGui::MenuItem("Create Empty Entity")) {
                        const Engine::Entity newEntity = registry->CreateEntity();
                        //registry->AddComponent<Engine::Position>(newEntity, Engine::Position(0, 0));
                        selected.entity = newEntity;
                        selected.index = static_cast<int>(newEntity); // Temporary simple selection map
                    }
                    ImGui::EndPopup();
                }

                if (registry->GetEntityCount() == 0) {
                    ImGui::TextDisabled("Right click to create an entity...");
                } else {
                    for (size_t i = 0; i < registry->GetEntityCount(); ++i) {
                        const auto entity = i;
                        std::string label = "Entity " + std::to_string(entity);
                        if (ImGui::Selectable(label.c_str(), selected.index == static_cast<int>(i))) {
                            selected.index = static_cast<int>(i);
                            selected.entity = entity;
                        }
                    }
                }
            }
        }
        ImGui::End();
    }
} // namespace Editor
