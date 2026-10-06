#include "Panels/HierarchyPanel.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/Position.hpp"
#include "Components/SpriteComponent.hpp"
#include <imgui.h>
#include <string>

namespace Editor {
void HierarchyPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    auto& selected = registry.GetResource<HierarchyPanelSelectedEntity>();
    if (ImGui::Begin(m_Name.c_str())) {
        // Right-click context menu in empty space
        if (ImGui::BeginPopupContextWindow("HierarchyContextMenu",
                                           ImGuiPopupFlags_MouseButtonRight |
                                               ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("Create Empty Entity")) {
                const Engine::Entity newEntity = registry.CreateEntity();
                // registry.AddComponent<Engine::Position>(newEntity, Engine::Position(0, 0));
                selected.entity = newEntity;
                selected.index = static_cast<int>(newEntity); // Temporary simple selection map
            }
            ImGui::EndPopup();
        }

        if (registry.GetEntityCount() == 0) {
            ImGui::TextDisabled("Right click to create an entity...");
        } else {
            for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
                const auto entity = i;
                std::string label = "Entity " + std::to_string(entity);
                if (ImGui::Selectable(label.c_str(), selected.index == static_cast<int>(i))) {
                    selected.index = static_cast<int>(i);
                    selected.entity = entity;
                }

                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload =
                            ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                        const char* path = (const char*)payload->Data;
                        std::string newPath(path);
                        try {
                            if (registry.HasResource<Engine::AssetManager>()) {
                                registry.GetResource<Engine::AssetManager>().LoadSprite(newPath,
                                                                                        newPath);
                            }
                            if (!registry.HasComponent<Engine::Position>(entity)) {
                                registry.AddComponent<Engine::Position>(entity,
                                                                        Engine::Position(0, 0));
                            }
                            if (registry.HasComponent<Engine::SpriteComponent>(entity)) {
                                registry.GetComponent<Engine::SpriteComponent>(entity).spriteAlias =
                                    newPath;
                            } else {
                                registry.AddComponent<Engine::SpriteComponent>(
                                    entity, Engine::SpriteComponent(newPath));
                            }
                        } catch (...) {
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }
        }

        // Window drag drop target
        ImGui::Dummy(ImGui::GetContentRegionAvail());
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                const char* path = (const char*)payload->Data;
                std::string newPath(path);
                try {
                    if (registry.HasResource<Engine::AssetManager>()) {
                        registry.GetResource<Engine::AssetManager>().LoadSprite(newPath, newPath);
                    }
                    const Engine::Entity newEntity = registry.CreateEntity();
                    registry.AddComponent<Engine::Position>(newEntity, Engine::Position(0, 0));
                    registry.AddComponent<Engine::SpriteComponent>(
                        newEntity, Engine::SpriteComponent(newPath));
                    selected.entity = newEntity;
                    selected.index = static_cast<int>(newEntity);
                } catch (...) {
                }
            }
            ImGui::EndDragDropTarget();
        }
    }
    ImGui::End();
}
} // namespace Editor

namespace Editor {
const std::string& HierarchyPanel::GetName() const {
    return m_Name;
}
} // namespace Editor
