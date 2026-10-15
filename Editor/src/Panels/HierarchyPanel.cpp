#include "Panels/HierarchyPanel.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/Name.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/SpriteComponent.hpp"
#include "EditorConfig.hpp"
#include "Logging/Logger.hpp"
#include "Panels/StatePanel.hpp"
#include <algorithm>
#include <cstring>
#include <extras/IconsFontAwesome6.h>
#include "Plugins/HierarchyPlugin.hpp"
#include "Systems/HierarchySystem.hpp"
#include <cmath>
#include <imgui.h>
#include <string>
#include <vector>

namespace Editor {
namespace {
// Detaches an entity from its parent (makes it a root). Clearing the
// relative flag: with no parent there is nothing to be relative to.
void DetachEntity(Engine::Registry& registry, Engine::Entity entity) {
    if (registry.HasComponent<Engine::Parent>(entity)) {
        registry.RemoveComponent<Engine::Parent>(entity);
    }
    if (registry.HasComponent<Engine::Position>(entity)) {
        registry.GetComponent<Engine::Position>(entity).relative = false;
    }
}

// Marks a subtree visited WITHOUT rendering it. Used for collapsed nodes:
// the top-level straggler sweep re-renders anything unvisited as a root,
// so an unmarked-but-hidden subtree would pop back into view when folded.
void MarkSubtreeVisited(Engine::Registry& registry, Engine::Entity entity,
                        std::unordered_set<Engine::Entity>& visited, int depth) {
    if (depth > 64 || !visited.insert(entity).second || !registry.IsEntityAlive(entity)) {
        return;
    }
    for (Engine::Entity child : Engine::HierarchySystem::FindDirectChildren(registry, entity)) {
        MarkSubtreeVisited(registry, child, visited, depth + 1);
    }
}
} // namespace

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
            const bool hasSelection = selected.entity != static_cast<Engine::Entity>(-1) &&
                                      registry.IsEntityAlive(selected.entity);
            if (!hasSelection) {
                ImGui::BeginDisabled();
            }
            if (ImGui::MenuItem("Remove Selected Entity")) {
                registry.DestroyEntity(selected.entity);
                selected.entity = static_cast<Engine::Entity>(-1);
                selected.index = -1;
            }
            if (!hasSelection) {
                ImGui::EndDisabled();
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

        if (ImGui::IsWindowFocused() && !ImGui::IsAnyItemActive() &&
            ImGui::IsKeyPressed(ImGuiKey_Delete) && selected.entity != -1 &&
            registry.IsEntityAlive(selected.entity)) {
            registry.DestroyEntity(selected.entity);
            selected.entity = static_cast<Engine::Entity>(-1);
            selected.index = -1;
        }

        ImGui::TextDisabled("Drag a row onto another to parent - drop on empty space to detach");
        ImGui::TextDisabled("Right-click a row for options - Del removes the selection");

        // Window drag drop target
        ImGui::Dummy(ImGui::GetContentRegionAvail());
        if (ImGui::BeginDragDropTarget()) {
            // Entity drop on empty space: detach, i.e. make it a root.
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_ID")) {
                const Engine::Entity dragged = *static_cast<const Engine::Entity*>(payload->Data);
                if (registry.IsEntityAlive(dragged)) {
                    DetachEntity(registry, dragged);
                }
            }
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
                    registry.AddComponent<Engine::Position>(newEntity, Engine::Position(0, 0));
                    registry.AddComponent<Engine::Rotation>(newEntity,
                                                            Engine::Rotation::Create(0.0f));
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
void HierarchyPanel::RenderEntityNode(Engine::Registry& registry, Engine::Entity entity,
                                      HierarchyPanelSelectedEntity& selected,
                                      std::unordered_set<Engine::Entity>& visited, int depth) {
    // Corrupt data (e.g. a hand-edited parent cycle) must never hang the UI.
    if (depth > 64 || !visited.insert(entity).second || !registry.IsEntityAlive(entity)) {
        return;
    }

    std::vector<Engine::Entity> children;
    for (Engine::Entity child : Engine::HierarchySystem::FindDirectChildren(registry, entity)) {
        if (registry.IsEntityAlive(child)) {
            children.push_back(child);
        }
    }

    const std::string name = Engine::GetEntityName(registry, entity);
    std::string label = name;
    if (!children.empty()) {
        label += " (" + std::to_string(children.size()) + ")";
    }
    // Stable imgui ID: ### resets the hash, so open/close state survives
    // display-text changes (child counts). With plain ## the full label is
    // hashed, and folding state is lost every time anything changes.
    label += "###" + std::to_string(entity);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (selected.entity == entity) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    const bool isLeaf = children.empty();
    if (isLeaf) {
        // NOTE: Leaf always reports open and pushes an ID scope (see
        // imgui.h), so TreePop() below stays balanced. Do NOT add
        // NoTreePushOnOpen here: it suppresses the push while open stays
        // true, which aborts in TreePop (IDStack underflow).
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    const bool open = ImGui::TreeNodeEx(label.c_str(), flags);

    // Arrow clicks toggle; anything else selects.
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        selected.index = static_cast<int>(entity);
        selected.entity = entity;
        if (registry.HasResource<StatePanelSelectedState>()) {
            auto& stateSel = registry.GetResource<StatePanelSelectedState>();
            stateSel.groupIndex = -1;
            stateSel.stateIndex = -1;
        }
    }

    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("ENTITY_ID", &entity, sizeof(entity));
        ImGui::TextUnformatted(name.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        // Entity drop: attach the dragged entity under this row.
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_ID")) {
            const Engine::Entity dragged = *static_cast<const Engine::Entity*>(payload->Data);
            if (dragged != entity && registry.IsEntityAlive(dragged)) {
                try {
                    Engine::HierarchyPlugin::AttachChild(registry, dragged, entity);
                } catch (const std::exception& e) {
                    log_error(std::string("[Hierarchy] attach failed: ") + e.what());
                }
            }
        }
        // Asset drop: attach/replace this row's sprite (existing behavior).
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
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
                    registry.AddComponent<Engine::Position>(entity, Engine::Position(0, 0));
                }
                if (!registry.HasComponent<Engine::Rotation>(entity)) {
                    registry.AddComponent<Engine::Rotation>(entity, Engine::Rotation::Create(0.0f));
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

    // Hierarchy details on hover: names stay clean, the parent link, live
    // offset and follow state show up here instead.
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) &&
        registry.HasComponent<Engine::Parent>(entity)) {
        const Engine::Entity parent = registry.GetComponent<Engine::Parent>(entity).parent;
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(("Parent: " + Engine::GetEntityName(registry, parent)).c_str());
        if (registry.HasComponent<Engine::Position>(entity) &&
            registry.HasComponent<Engine::Position>(parent)) {
            const auto& cp = registry.GetComponent<Engine::Position>(entity);
            const auto& pp = registry.GetComponent<Engine::Position>(parent);
            ImGui::Text("Offset: (%+d, %+d)", static_cast<int>(std::lround(cp.x - pp.x)),
                        static_cast<int>(std::lround(cp.y - pp.y)));
        }
        if (registry.HasComponent<Engine::Position>(entity)) {
            ImGui::Text("Follow parent: %s",
                        registry.GetComponent<Engine::Position>(entity).relative ? "on" : "off");
        }
        ImGui::EndTooltip();
    }

    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Remove Entity")) {
            registry.DestroyEntity(entity);
            if (selected.entity == entity || !registry.IsEntityAlive(selected.entity)) {
                selected.entity = static_cast<Engine::Entity>(-1);
                selected.index = -1;
            }
        }
        const bool canReparent = selected.entity != static_cast<Engine::Entity>(-1) &&
                                 selected.entity != entity &&
                                 registry.IsEntityAlive(selected.entity);
        if (!canReparent) {
            ImGui::BeginDisabled();
        }
        if (ImGui::MenuItem("Make Child Of Selected")) {
            try {
                Engine::HierarchyPlugin::AttachChild(registry, entity, selected.entity);
            } catch (const std::exception& e) {
                log_error(std::string("[Hierarchy] attach failed: ") + e.what());
            }
        }
        if (!canReparent) {
            ImGui::EndDisabled();
        }
        const bool hasParent = registry.HasComponent<Engine::Parent>(entity);
        if (!hasParent) {
            ImGui::BeginDisabled();
        }
        if (ImGui::MenuItem("Detach From Parent")) {
            DetachEntity(registry, entity);
        }
        if (!hasParent) {
            ImGui::EndDisabled();
        }
        // Recovery/layout helper: place the child exactly on its parent
        // (offset zero). Useful after the pre-fix fly-away corrupted offsets.
        const bool canSnap = hasParent && registry.HasComponent<Engine::Position>(entity) &&
                             registry.HasComponent<Engine::Position>(
                                 registry.GetComponent<Engine::Parent>(entity).parent);
        if (!canSnap) {
            ImGui::BeginDisabled();
        }
        if (ImGui::MenuItem("Snap To Parent")) {
            const Engine::Entity parent = registry.GetComponent<Engine::Parent>(entity).parent;
            auto& cp = registry.GetComponent<Engine::Position>(entity);
            const auto& pp = registry.GetComponent<Engine::Position>(parent);
            cp.x = pp.x;
            cp.y = pp.y;
        }
        if (!canSnap) {
            ImGui::EndDisabled();
        }
        ImGui::EndPopup();
    }

    if (open) {
        for (Engine::Entity child : children) {
            RenderEntityNode(registry, child, selected, visited, depth + 1);
        }
        ImGui::TreePop();
    } else {
        // Still mark the hidden subtree so the straggler sweep below does
        // not resurrect it as root rows.
        for (Engine::Entity child : children) {
            MarkSubtreeVisited(registry, child, visited, depth + 1);
        }
    }
}

const std::string& HierarchyPanel::GetName() const {
    return m_Name;
}
} // namespace Editor
