#include "Panels/HierarchyPanel.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/Name.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/SpriteComponent.hpp"
#include "Panels/StatePanel.hpp"
#include "EditorConfig.hpp"
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
                selected.entity = newEntity;
                selected.index = static_cast<int>(newEntity); // Temporary simple selection map
                if (registry.HasResource<StatePanelSelectedState>()) {
                    auto& stateSel = registry.GetResource<StatePanelSelectedState>();
                    stateSel.groupIndex = -1;
                    stateSel.stateIndex = -1;
                }
            }
            ImGui::EndPopup();
        }

        if (registry.GetEntityCount() == 0) {
            ImGui::TextDisabled("Right click to create an entity...");
        } else {
            for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
                const auto entity = i;
                std::string label =
                    Engine::GetEntityName(registry, entity) + "##" + std::to_string(entity);
                if (ImGui::Selectable(label.c_str(), selected.index == static_cast<int>(i))) {
                    selected.index = static_cast<int>(i);
                    selected.entity = entity;
                    if (registry.HasResource<StatePanelSelectedState>()) {
                        auto& stateSel = registry.GetResource<StatePanelSelectedState>();
                        stateSel.groupIndex = -1;
                        stateSel.stateIndex = -1;
                    }
                }

                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload =
                            ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                        const char* path = (const char*)payload->Data;
                        std::string rawPath(path);
                        std::string relPath = ToProjectRelativePath(registry, rawPath);
                        auto resolvedPath = ResolveAssetPath(registry, relPath);
                        try {
                            std::shared_ptr<Engine::Sprite> spr = nullptr;
                            if (registry.HasResource<Engine::AssetManager>()) {
                                auto& am = registry.GetResource<Engine::AssetManager>();
                                try {
                                    auto existing = am.GetSprite(relPath);
                                    if (existing && existing->GetTexture().id == 0) {
                                        am.UnloadSprite(relPath);
                                    }
                                } catch (...) {
                                }
                                am.LoadSprite(relPath, resolvedPath.string());
                                spr = am.GetSprite(relPath);
                            } else {
                                spr = std::make_shared<Engine::Sprite>(resolvedPath.string());
                            }
                            if (!registry.HasComponent<Engine::Position>(entity)) {
                                registry.AddComponent<Engine::Position>(entity,
                                                                        Engine::Position(0, 0, 0));
                            }
                            if (!registry.HasComponent<Engine::Rotation>(entity)) {
                                registry.AddComponent<Engine::Rotation>(entity,
                                                                        Engine::Rotation(0.0f));
                            }
                            if (registry.HasComponent<Engine::SpriteComponent>(entity)) {
                                auto& sc = registry.GetComponent<Engine::SpriteComponent>(entity);
                                sc.sprite = spr;
                                sc.assetPath = relPath;
                            } else {
                                registry.AddComponent<Engine::SpriteComponent>(
                                    entity, Engine::SpriteComponent(spr, relPath));
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
                std::string rawPath(path);
                std::string relPath = ToProjectRelativePath(registry, rawPath);
                auto resolvedPath = ResolveAssetPath(registry, relPath);
                try {
                    std::shared_ptr<Engine::Sprite> spr = nullptr;
                    if (registry.HasResource<Engine::AssetManager>()) {
                        auto& am = registry.GetResource<Engine::AssetManager>();
                        try {
                            auto existing = am.GetSprite(relPath);
                            if (existing && existing->GetTexture().id == 0) {
                                am.UnloadSprite(relPath);
                            }
                        } catch (...) {
                        }
                        am.LoadSprite(relPath, resolvedPath.string());
                        spr = am.GetSprite(relPath);
                    } else {
                        spr = std::make_shared<Engine::Sprite>(resolvedPath.string());
                    }
                    const Engine::Entity newEntity = registry.CreateEntity();
                    registry.AddComponent<Engine::Position>(newEntity, Engine::Position(0, 0, 0));
                    registry.AddComponent<Engine::Rotation>(newEntity, Engine::Rotation(0.0f));
                    registry.AddComponent<Engine::SpriteComponent>(
                        newEntity, Engine::SpriteComponent(spr, relPath));
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
