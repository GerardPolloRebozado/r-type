#include "Panels/HierarchyPanel.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/Name.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/SpriteComponent.hpp"
#include "EditorConfig.hpp"
#include "EntitySubtree.hpp"
#include "Logging/Logger.hpp"
#include "Panels/StatePanel.hpp"
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
        registry.RequireComponent<Engine::Position>(entity).relative = false;
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

bool IsPrefabPath(const std::string& path) {
    return path.size() >= 7 && path.ends_with(".prefab");
}

void SelectSpawned(Engine::Registry& registry, HierarchyPanelSelectedEntity& selected,
                   Engine::Entity entity) {
    selected.entity = entity;
    selected.index = static_cast<int>(entity);
    if (registry.HasResource<StatePanelSelectedState>()) {
        auto& stateSel = registry.GetResource<StatePanelSelectedState>();
        stateSel.groupIndex = -1;
        stateSel.stateIndex = -1;
    }
}

void PasteClipboard(Engine::Registry& registry, HierarchyPanelSelectedEntity& selected) {
    const Engine::Entity pasted = EntitySubtree::Paste(registry);
    if (pasted != static_cast<Engine::Entity>(-1)) {
        SelectSpawned(registry, selected, pasted);
    }
}
} // namespace

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
            const bool hasSelection = selected.entity != static_cast<Engine::Entity>(-1) &&
                                      registry.IsEntityAlive(selected.entity);
            if (!hasSelection) {
                ImGui::BeginDisabled();
            }
            if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                EntitySubtree::Copy(registry, selected.entity);
            }
            if (!hasSelection) {
                ImGui::EndDisabled();
            }
            if (!EntitySubtree::CanPaste()) {
                ImGui::BeginDisabled();
            }
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                PasteClipboard(registry, selected);
            }
            if (!EntitySubtree::CanPaste()) {
                ImGui::EndDisabled();
            }
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

        if (registry.GetEntityCount() == 0) {
            ImGui::TextDisabled("Right click to create an entity...");
        } else {
            // Roots: alive entities with no live parent link. IDs are never
            // reused and dead rows are skipped, so removal hides rows.
            std::unordered_set<Engine::Entity> visited;
            for (size_t i = 1; i < registry.GetEntityCount(); ++i) {
                const Engine::Entity entity = i;
                if (!registry.IsEntityAlive(entity)) {
                    continue;
                }
                const bool hasLiveParent =
                    registry.HasComponent<Engine::Parent>(entity) &&
                    registry.IsEntityAlive(registry.RequireComponent<Engine::Parent>(entity).parent);
                if (!hasLiveParent) {
                    RenderEntityNode(registry, entity, selected, visited, 0);
                }
            }
            // Stragglers: alive but unreachable from any root (e.g. a
            // hand-edited parent cycle). Rendered as roots so nothing hides.
            for (size_t i = 1; i < registry.GetEntityCount(); ++i) {
                const Engine::Entity entity = i;
                if (registry.IsEntityAlive(entity) && !visited.contains(entity)) {
                    RenderEntityNode(registry, entity, selected, visited, 0);
                }
            }
        }

        if (ImGui::IsWindowFocused() && !ImGui::IsAnyItemActive() &&
            ImGui::IsKeyPressed(ImGuiKey_Delete) && selected.entity != -1 &&
            registry.IsEntityAlive(selected.entity)) {
            registry.DestroyEntity(selected.entity);
            selected.entity = static_cast<Engine::Entity>(-1);
            selected.index = -1;
        }

        // Only while this window is focused and no text item is active, so
        // fields in other panels keep Ctrl+C / Ctrl+V.
        if (ImGui::IsWindowFocused() && !ImGui::IsAnyItemActive()) {
            const ImGuiIO& io = ImGui::GetIO();
            if (io.KeyCtrl && !io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_C, false) &&
                selected.entity != static_cast<Engine::Entity>(-1) &&
                registry.IsEntityAlive(selected.entity)) {
                EntitySubtree::Copy(registry, selected.entity);
            }
            if (io.KeyCtrl && !io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_V, false) &&
                EntitySubtree::CanPaste()) {
                PasteClipboard(registry, selected);
            }
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
                const char* path = (const char*)payload->Data;
                std::string rawPath(path);
                // A .prefab is a subtree, not a texture. The sprite path below
                // would try to load the JSON as an image.
                if (IsPrefabPath(rawPath)) {
                    const Engine::Entity spawned = EntitySubtree::SpawnPrefab(registry, rawPath);
                    if (spawned != static_cast<Engine::Entity>(-1)) {
                        SelectSpawned(registry, selected, spawned);
                    }
                } else {
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
                        selected.entity = newEntity;
                        selected.index = static_cast<int>(newEntity);
                    } catch (...) {
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
    }
    ImGui::End();
}
} // namespace Editor

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
                    log_user_error(std::string("[Hierarchy] attach failed: ") + e.what());
                }
            }
        }
        // Asset drop: attach/replace this row's sprite (existing behavior).
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
            const char* path = (const char*)payload->Data;
            std::string rawPath(path);
            // A .prefab drop spawns a subtree. It must not replace this row's sprite.
            if (IsPrefabPath(rawPath)) {
                const Engine::Entity spawned = EntitySubtree::SpawnPrefab(registry, rawPath);
                if (spawned != static_cast<Engine::Entity>(-1)) {
                    SelectSpawned(registry, selected, spawned);
                }
            } else {
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
                        registry.AddComponent<Engine::Rotation>(entity,
                                                                Engine::Rotation::Create(0.0f));
                    }
                    if (registry.HasComponent<Engine::SpriteComponent>(entity)) {
                        auto& sc = registry.RequireComponent<Engine::SpriteComponent>(entity);
                        sc.sprite = spr;
                        sc.assetPath = relPath;
                    } else {
                        registry.AddComponent<Engine::SpriteComponent>(
                            entity, Engine::SpriteComponent(spr, relPath));
                    }
                } catch (...) {
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    // Hierarchy details on hover: names stay clean, the parent link, live
    // offset and follow state show up here instead.
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) &&
        registry.HasComponent<Engine::Parent>(entity)) {
        const Engine::Entity parent = registry.RequireComponent<Engine::Parent>(entity).parent;
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(("Parent: " + Engine::GetEntityName(registry, parent)).c_str());
        if (registry.HasComponent<Engine::Position>(entity) &&
            registry.HasComponent<Engine::Position>(parent)) {
            const auto& cp = registry.RequireComponent<Engine::Position>(entity);
            const auto& pp = registry.RequireComponent<Engine::Position>(parent);
            ImGui::Text("Offset: (%+d, %+d)", static_cast<int>(std::lround(cp.x - pp.x)),
                        static_cast<int>(std::lround(cp.y - pp.y)));
        }
        if (registry.HasComponent<Engine::Position>(entity)) {
            ImGui::Text("Follow parent: %s",
                        registry.RequireComponent<Engine::Position>(entity).relative ? "on" : "off");
        }
        ImGui::EndTooltip();
    }

    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Copy", "Ctrl+C")) {
            EntitySubtree::Copy(registry, entity);
        }
        if (!EntitySubtree::CanPaste()) {
            ImGui::BeginDisabled();
        }
        if (ImGui::MenuItem("Paste", "Ctrl+V")) {
            PasteClipboard(registry, selected);
        }
        if (!EntitySubtree::CanPaste()) {
            ImGui::EndDisabled();
        }
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
                log_user_error(std::string("[Hierarchy] attach failed: ") + e.what());
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
        // Re-check Parent: Detach above removes it, and the earlier hasParent
        // flag would still be true, so GetComponent would throw out_of_range.
        const bool canSnap = registry.HasComponent<Engine::Parent>(entity) &&
                             registry.HasComponent<Engine::Position>(entity) &&
                             registry.HasComponent<Engine::Position>(
                                 registry.RequireComponent<Engine::Parent>(entity).parent);
        if (!canSnap) {
            ImGui::BeginDisabled();
        }
        if (ImGui::MenuItem("Snap To Parent")) {
            const Engine::Entity parent = registry.RequireComponent<Engine::Parent>(entity).parent;
            auto& cp = registry.RequireComponent<Engine::Position>(entity);
            const auto& pp = registry.RequireComponent<Engine::Position>(parent);
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
