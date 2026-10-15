#include "Panels/HierarchyPanel.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/Name.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/SpriteComponent.hpp"
#include "EditorConfig.hpp"
#include "Panels/StatePanel.hpp"
#include <algorithm>
#include <cstring>
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <string>

namespace Editor {
void HierarchyPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    if (!registry.HasResource<HierarchyPanelSelectedEntity>()) {
        registry.AddResource<HierarchyPanelSelectedEntity>();
    }
    if (!registry.HasResource<HierarchyPanelSelectedDirectory>()) {
        registry.AddResource<HierarchyPanelSelectedDirectory>();
    }
    if (!registry.HasResource<EntityDirectoriesManager>()) {
        registry.AddResource<EntityDirectoriesManager>();
    } //

    auto& selected = registry.GetResource<HierarchyPanelSelectedEntity>();
    auto& selectedDir = registry.GetResource<HierarchyPanelSelectedDirectory>();
    auto& dirManager = registry.GetResource<EntityDirectoriesManager>();

    if (dirManager.directories.empty()) {
        dirManager.directories.push_back({"/", "root", {}});
    }

    Engine::Entity entityToDelete = static_cast<Engine::Entity>(-1);

    if (ImGui::Begin(m_Name.c_str())) {
        // Right-click context menu in empty space
        if (ImGui::BeginPopupContextWindow("HierarchyContextMenu",
                                           ImGuiPopupFlags_MouseButtonRight |
                                               ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem(ICON_FA_FOLDER_PLUS "  Create Folder")) {
                std::string folderName = "Folder_" + std::to_string(dirManager.directories.size());
                dirManager.directories.push_back({"/" + folderName, folderName, {}});
                selectedDir.directoryIndex = static_cast<int>(dirManager.directories.size() - 1);
                selected.entity = -1;
                selected.index = -1;
                if (registry.HasResource<StatePanelSelectedState>()) {
                    auto& stateSel = registry.GetResource<StatePanelSelectedState>();
                    stateSel.groupIndex = -1;
                    stateSel.stateIndex = -1;
                }
            }
            if (ImGui::MenuItem(ICON_FA_CUBE "  Create Empty Entity")) {
                const Engine::Entity newEntity = registry.CreateEntity();
                dirManager.directories[0].entities.push_back(newEntity);
                selected.entity = newEntity;
                selected.index = static_cast<int>(newEntity);
                selectedDir.directoryIndex = -1;
                if (registry.HasResource<StatePanelSelectedState>()) {
                    auto& stateSel = registry.GetResource<StatePanelSelectedState>();
                    stateSel.groupIndex = -1;
                    stateSel.stateIndex = -1;
                }
            }
            ImGui::EndPopup();
        }

        for (size_t dirIdx = 0; dirIdx < dirManager.directories.size(); ++dirIdx) {
            auto& dir = dirManager.directories[dirIdx];
            ImGui::PushID(static_cast<int>(dirIdx));

            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow |
                                           ImGuiTreeNodeFlags_SpanAvailWidth |
                                           ImGuiTreeNodeFlags_DefaultOpen;
            if (selectedDir.directoryIndex == static_cast<int>(dirIdx) && selected.entity == -1) {
                nodeFlags |= ImGuiTreeNodeFlags_Selected;
            }

            const bool nodeOpen =
                ImGui::TreeNodeEx("##DirNode", nodeFlags, "%s  %s (%zu)", ICON_FA_FOLDER,
                                  dir.Name.c_str(), dir.entities.size());

            if (ImGui::IsItemClicked()) {
                selectedDir.directoryIndex = static_cast<int>(dirIdx);
                selected.entity = -1;
                selected.index = -1;
                if (registry.HasResource<StatePanelSelectedState>()) {
                    auto& stateSel = registry.GetResource<StatePanelSelectedState>();
                    stateSel.groupIndex = -1;
                    stateSel.stateIndex = -1;
                }
            }

            // Right-click context menu on folder
            if (ImGui::BeginPopupContextItem("FolderContextMenu")) {
                if (ImGui::MenuItem(ICON_FA_CUBE "  Create Entity Here")) {
                    const Engine::Entity newEntity = registry.CreateEntity();
                    dir.entities.push_back(newEntity);
                    selected.entity = newEntity;
                    selected.index = static_cast<int>(newEntity);
                    selectedDir.directoryIndex = static_cast<int>(dirIdx);
                    if (registry.HasResource<StatePanelSelectedState>()) {
                        auto& stateSel = registry.GetResource<StatePanelSelectedState>();
                        stateSel.groupIndex = -1;
                        stateSel.stateIndex = -1;
                    }
                }
                if (dirIdx > 0) {
                    if (ImGui::MenuItem("Delete Folder")) {
                        for (auto ent : dir.entities) {
                            dirManager.directories[0].entities.push_back(ent);
                        }
                        dirManager.directories.erase(dirManager.directories.begin() + dirIdx);
                        if (selectedDir.directoryIndex == static_cast<int>(dirIdx)) {
                            selectedDir.directoryIndex = -1;
                        } else if (selectedDir.directoryIndex > static_cast<int>(dirIdx)) {
                            selectedDir.directoryIndex--;
                        }
                        ImGui::EndPopup();
                        if (nodeOpen) {
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                        break;
                    }
                }
                ImGui::EndPopup();
            }

            if (nodeOpen) {
                if (dir.entities.empty()) {
                    ImGui::TextDisabled("   (empty)");
                } else {
                    for (auto entity : dir.entities) {
                        ImGui::PushID(static_cast<int>(entity));

                        std::string label = std::string(ICON_FA_CUBE " ") +
                                            Engine::GetEntityName(registry, entity) + "##" +
                                            std::to_string(entity);

                        if (ImGui::Selectable(label.c_str(), selected.entity == entity)) {
                            selected.entity = entity;
                            selected.index = static_cast<int>(entity);
                            selectedDir.directoryIndex = -1;
                            if (registry.HasResource<StatePanelSelectedState>()) {
                                auto& stateSel = registry.GetResource<StatePanelSelectedState>();
                                stateSel.groupIndex = -1;
                                stateSel.stateIndex = -1;
                            }
                        }

                        if (ImGui::BeginPopupContextItem("EntityContextMenu")) {
                            if (ImGui::MenuItem("Delete Entity")) {
                                entityToDelete = entity;
                            }
                            ImGui::EndPopup();
                        }

                        if (ImGui::BeginDragDropTarget()) {
                            if (const ImGuiPayload* payload =
                                    ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                                const char* path = static_cast<const char*>(payload->Data);
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
                                        spr =
                                            std::make_shared<Engine::Sprite>(resolvedPath.string());
                                    }
                                    if (!registry.HasComponent<Engine::Position>(entity)) {
                                        registry.AddComponent<Engine::Position>(
                                            entity, Engine::Position(0, 0));
                                    }
                                    if (!registry.HasComponent<Engine::Rotation>(entity)) {
                                        registry.AddComponent<Engine::Rotation>(
                                            entity, Engine::Rotation(0.0f));
                                    }
                                    if (registry.HasComponent<Engine::SpriteComponent>(entity)) {
                                        auto& sc =
                                            registry.GetComponent<Engine::SpriteComponent>(entity);
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

                        ImGui::PopID();
                    }
                }
                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        if (entityToDelete != static_cast<Engine::Entity>(-1)) {
            registry.DestroyEntity(entityToDelete);
            for (auto& dir : dirManager.directories) {
                dir.entities.erase(
                    std::remove(dir.entities.begin(), dir.entities.end(), entityToDelete),
                    dir.entities.end());
            }
            if (selected.entity == entityToDelete) {
                selected.entity = -1;
                selected.index = -1;
            }
        }

        // Window drag drop target
        ImGui::Dummy(ImGui::GetContentRegionAvail());
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                const char* path = static_cast<const char*>(payload->Data);
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
                    dirManager.directories[0].entities.push_back(newEntity);
                    selected.entity = newEntity;
                    selected.index = static_cast<int>(newEntity);
                    selectedDir.directoryIndex = -1;
                    if (registry.HasResource<StatePanelSelectedState>()) {
                        auto& stateSel = registry.GetResource<StatePanelSelectedState>();
                        stateSel.groupIndex = -1;
                        stateSel.stateIndex = -1;
                    }
                } catch (...) {
                }
            }
            ImGui::EndDragDropTarget();
        }
    }
    ImGui::End();
}
}

namespace Editor {
const std::string& HierarchyPanel::GetName() const {
    return m_Name;
}
} // namespace Editor
