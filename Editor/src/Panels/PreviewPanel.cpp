#include "Panels/PreviewPanel.hpp"
#include "Components/AudioSource.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Input.hpp"
#include "Components/Name.hpp"
#include "Components/NetworkComponent.hpp"
#include "Components/Opacity.hpp"
#include "Components/Parent.hpp"
#include "Components/PrefabInstance.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "Components/SignalConnections.hpp"
#include "Components/SpriteComponent.hpp"
#include "Components/Text.hpp"
#include "EditorConfig.hpp"
#include "EntitySubtree.hpp"
#include "GlazeSerializer.hpp"
#include "Logging/Logger.hpp"
#include "Plugins/AudioPluginEditor.hpp"
#include "Plugins/CameraPluginEditor.hpp"
#include "Plugins/ColliderPluginEditor.hpp"
#include "Plugins/NetworkComponentPluginEditor.hpp"
#include "Plugins/OpacityPluginEditor.hpp"
#include "Plugins/PositionPluginEditor.hpp"
#include "Plugins/RotationPluginEditor.hpp"
#include "Plugins/ScriptPluginEditor.hpp"
#include "Plugins/ShapePluginEditor.hpp"
#include "Plugins/SpritePluginEditor.hpp"
#include "Plugins/TextPluginEditor.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Systems/HierarchySystem.hpp"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <imgui.h>
#include <memory>
#include <optional>
#include <rlImGui.h>
#include <unordered_set>

namespace Editor {
using Snapshot = Serialization::GlazeSerializer::EntitySnapshot;

namespace {
bool IsImageFile(const std::string& path) {
    std::string ext = std::filesystem::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".gif" ||
           ext == ".tga";
}

bool IsPrefabFile(const std::string& path) {
    std::string ext = std::filesystem::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == ".prefab";
}

void RegisterPreviewComponents(Engine::Registry& registry) {
    registry.RegisterComponent<Engine::Name>();
    registry.RegisterComponent<Engine::SpriteComponent>();
    registry.RegisterComponent<Engine::ShapeComponent>();
    registry.RegisterComponent<Engine::Text>();
    registry.RegisterComponent<Engine::ColliderComponent>();
    registry.RegisterComponent<Engine::Opacity>();
    registry.RegisterComponent<Engine::Parent>();
    registry.RegisterComponent<Engine::Input>();
    registry.RegisterComponent<Engine::AudioSource>();
    registry.RegisterComponent<Engine::CameraComponent>();
    registry.RegisterComponent<Engine::ScriptComponent>();
    registry.RegisterComponent<Engine::SignalConnections>();
    registry.RegisterComponent<Engine::PrefabInstance>();
    registry.RegisterComponent<Engine::NetworkComponent>();
}

void CollectSubtree(Engine::Registry& registry, Engine::Entity entity,
                    Serialization::GlazeSerializer& serializer, std::vector<Snapshot>& out,
                    std::unordered_set<Engine::Entity>& seen, int depth) {
    if (depth > 64 || !seen.insert(entity).second || !registry.IsEntityAlive(entity)) {
        return;
    }
    out.push_back(serializer.CaptureEntity(registry, entity));
    std::vector<Engine::Entity> children =
        Engine::HierarchySystem::FindDirectChildren(registry, entity);
    std::sort(children.begin(), children.end());
    for (Engine::Entity child : children) {
        CollectSubtree(registry, child, serializer, out, seen, depth + 1);
    }
}

std::vector<Snapshot> CaptureSubtree(Engine::Registry& registry, Engine::Entity root) {
    std::vector<Snapshot> snapshots;
    std::unordered_set<Engine::Entity> seen;
    Serialization::GlazeSerializer serializer;
    CollectSubtree(registry, root, serializer, snapshots, seen, 0);
    return snapshots;
}

void CollectEntities(Engine::Registry& registry, Engine::Entity entity,
                     std::vector<Engine::Entity>& out, std::unordered_set<Engine::Entity>& seen,
                     int depth) {
    if (depth > 64 || !seen.insert(entity).second || !registry.IsEntityAlive(entity)) {
        return;
    }
    out.push_back(entity);
    std::vector<Engine::Entity> children =
        Engine::HierarchySystem::FindDirectChildren(registry, entity);
    std::sort(children.begin(), children.end());
    for (Engine::Entity child : children) {
        CollectEntities(registry, child, out, seen, depth + 1);
    }
}

std::string NormalizeStoredPath(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    if (path.rfind("./", 0) == 0) {
        path.erase(0, 2);
    }
    return path;
}

bool SamePrefab(Engine::Registry& scene, const std::string& stored, const std::string& relative,
                const std::string& absolutePath) {
    if (stored.empty()) {
        return false;
    }
    if (NormalizeStoredPath(stored) == NormalizeStoredPath(relative)) {
        return true;
    }
    const std::filesystem::path storedFile = ResolveAssetPath(scene, stored);
    std::error_code error;
    return std::filesystem::equivalent(storedFile, absolutePath, error);
}

void ApplySprite(Engine::Registry& scene, Engine::Entity entity,
                 const Engine::SpriteComponent& source) {
    scene.RegisterComponent<Engine::SpriteComponent>();
    if (!scene.HasComponent<Engine::SpriteComponent>(entity)) {
        scene.AddComponent<Engine::SpriteComponent>(
            entity, Engine::SpriteComponent(nullptr, "", 0.0f, 0.0f, source.scaleX, source.scaleY));
    }
    auto& sprite = scene.GetComponent<Engine::SpriteComponent>(entity);
    if (sprite.assetPath != source.assetPath || sprite.sprite == nullptr) {
        LoadSpriteForComponent(scene, sprite, source.assetPath);
    }
    sprite.offsetX = source.offsetX;
    sprite.offsetY = source.offsetY;
    sprite.scaleX = source.scaleX;
    sprite.scaleY = source.scaleY;
    sprite.flipX = source.flipX;
    sprite.flipY = source.flipY;
    if (sprite.sprite) {
        sprite.sprite->SetScale({sprite.scaleX, sprite.scaleY});
    }
}

template <typename T> void KeepIfMissing(std::optional<T>& dest, const std::optional<T>& fallback) {
    if (!dest.has_value()) {
        dest = fallback;
    }
}

template <typename T>
void ApplyCopy(Engine::Registry& scene, Engine::Entity entity, const T& source) {
    scene.RegisterComponent<T>();
    if (!scene.HasComponent<T>(entity)) {
        scene.AddComponent<T>(entity, source);
        return;
    }
    scene.GetComponent<T>(entity) = source;
}

void ApplyScript(Engine::Registry& scene, Engine::Entity entity,
                 const Engine::ScriptComponent& source) {
    scene.RegisterComponent<Engine::ScriptComponent>();
    if (!scene.HasComponent<Engine::ScriptComponent>(entity)) {
        scene.AddComponent<Engine::ScriptComponent>(
            entity, Engine::ScriptComponent(source.scriptPath, source.autoReload));
        return;
    }
    auto& script = scene.GetComponent<Engine::ScriptComponent>(entity);
    if (script.scriptPath == source.scriptPath) {
        return;
    }
    script.scriptPath = source.scriptPath;
    script.autoReload = source.autoReload;
    script.loaded = false;
    script.scriptTable = sol::lua_nil;
}

void ApplyText(Engine::Registry& scene, Engine::Entity entity, const Engine::Text& source) {
    scene.RegisterComponent<Engine::Text>();
    if (!scene.HasComponent<Engine::Text>(entity)) {
        scene.AddComponent<Engine::Text>(entity,
                                         Engine::Text(source.text, source.fontsize, source.color));
        auto& text = scene.GetComponent<Engine::Text>(entity);
        text.spacing = source.spacing;
        return;
    }
    auto& text = scene.GetComponent<Engine::Text>(entity);
    text.text = source.text;
    text.fontsize = source.fontsize;
    text.spacing = source.spacing;
    text.color = source.color;
}

void ApplyNetwork(Engine::Registry& scene, Engine::Entity entity,
                  const Engine::NetworkComponent& source) {
    scene.RegisterComponent<Engine::NetworkComponent>();
    if (!scene.HasComponent<Engine::NetworkComponent>(entity)) {
        scene.AddComponent<Engine::NetworkComponent>(entity, source);
        return;
    }
    auto& network = scene.GetComponent<Engine::NetworkComponent>(entity);
    const std::uint64_t networkId = network.networkId;
    network = source;
    network.networkId = networkId;
}

void ApplyCamera(Engine::Registry& scene, Engine::Entity entity,
                 const Engine::CameraComponent& source) {
    scene.RegisterComponent<Engine::CameraComponent>();
    if (!scene.HasComponent<Engine::CameraComponent>(entity)) {
        Engine::CameraComponent copy = source;
        copy.followTarget.reset();
        scene.AddComponent<Engine::CameraComponent>(entity, copy);
        return;
    }
    auto& camera = scene.GetComponent<Engine::CameraComponent>(entity);
    camera.zoom = source.zoom;
    camera.primary = source.primary;
    camera.followSpeed = source.followSpeed;
    camera.clampToBounds = source.clampToBounds;
    camera.bounds = source.bounds;
    camera.shakeMagnitude = source.shakeMagnitude;
    camera.shakeDecay = source.shakeDecay;
}

void ApplyChildPosition(Engine::Registry& scene, Engine::Entity entity,
                        const std::vector<Snapshot>& snapshots, std::size_t index) {
    const Snapshot& source = snapshots[index];
    if (!source.position.has_value() || !source.parent.has_value()) {
        return;
    }
    std::size_t parentIndex = snapshots.size();
    for (std::size_t i = 0; i < snapshots.size(); ++i) {
        if (snapshots[i].entityId == source.parent->parent) {
            parentIndex = i;
            break;
        }
    }
    if (parentIndex >= snapshots.size() || !snapshots[parentIndex].position.has_value()) {
        return;
    }
    scene.RegisterComponent<Engine::Position>();
    if (!scene.HasComponent<Engine::Position>(entity) ||
        !scene.HasComponent<Engine::Parent>(entity)) {
        return;
    }
    const Engine::Entity sceneParent = scene.GetComponent<Engine::Parent>(entity).parent;
    if (!scene.IsEntityAlive(sceneParent) || !scene.HasComponent<Engine::Position>(sceneParent)) {
        return;
    }

    const Engine::Position& prefabParent = *snapshots[parentIndex].position;
    const Engine::Position& prefabChild = *source.position;
    auto& dest = scene.GetComponent<Engine::Position>(entity);
    const auto& instanceParent = scene.GetComponent<Engine::Position>(sceneParent);
    dest.x = instanceParent.x + (prefabChild.x - prefabParent.x);
    dest.y = instanceParent.y + (prefabChild.y - prefabParent.y);
    dest.z = prefabChild.z;
    dest.relative = prefabChild.relative;
    dest.prevX = dest.x;
    dest.prevY = dest.y;
    dest.prevZ = dest.z;
    dest.moved = false;
}

void DrawAddComponentMenu(std::vector<std::unique_ptr<IPluginEditor>>& editors,
                          Engine::Entity entity, Engine::Registry& registry) {
    bool anyAvailable = false;
    for (auto& editor : editors) {
        if (!editor->HasComponent(entity, registry)) {
            anyAvailable = true;
            const std::string menuItem =
                std::string(editor->GetIcon()) + " Add " + editor->GetName();
            if (ImGui::MenuItem(menuItem.c_str())) {
                editor->AddComponent(entity, registry);
            }
        }
    }
    if (!anyAvailable) {
        ImGui::TextDisabled("All plugins added");
    }
}

void LoadPrefabSprites(Engine::Registry& registry, Engine::Entity entity,
                       std::unordered_set<Engine::Entity>& seen, int depth) {
    if (depth > 64 || !seen.insert(entity).second || !registry.IsEntityAlive(entity)) {
        return;
    }
    if (registry.HasComponent<Engine::SpriteComponent>(entity)) {
        auto& sprite = registry.GetComponent<Engine::SpriteComponent>(entity);
        if (sprite.sprite == nullptr && !sprite.assetPath.empty()) {
            LoadSpriteForComponent(registry, sprite, sprite.assetPath);
        }
    }
    std::vector<Engine::Entity> children =
        Engine::HierarchySystem::FindDirectChildren(registry, entity);
    std::sort(children.begin(), children.end());
    for (Engine::Entity child : children) {
        LoadPrefabSprites(registry, child, seen, depth + 1);
    }
}

void ApplySnapshot(Engine::Registry& scene, Engine::Entity entity, const Snapshot& source) {
    if (source.sprite.has_value()) {
        ApplySprite(scene, entity, *source.sprite);
    }
    if (source.shape.has_value()) {
        ApplyCopy(scene, entity, *source.shape);
    }
    if (source.script.has_value()) {
        ApplyScript(scene, entity, *source.script);
    }
    if (source.network.has_value()) {
        ApplyNetwork(scene, entity, *source.network);
    }
    if (source.collider.has_value()) {
        ApplyCopy(scene, entity, *source.collider);
    }
    if (source.text.has_value()) {
        ApplyText(scene, entity, *source.text);
    }
    if (source.opacity.has_value()) {
        ApplyCopy(scene, entity, *source.opacity);
    }
    if (source.rotation.has_value()) {
        ApplyCopy(scene, entity, *source.rotation);
    }
    if (source.camera.has_value()) {
        ApplyCamera(scene, entity, *source.camera);
    }
    if (source.audioSource.has_value()) {
        scene.RegisterComponent<Engine::AudioSource>();
        if (scene.HasComponent<Engine::AudioSource>(entity)) {
            scene.GetComponent<Engine::AudioSource>(entity) = *source.audioSource;
        } else {
            scene.AddComponent(entity, *source.audioSource);
        }
    }
}
} // namespace

struct PreviewPanel::PrefabPreviewState {
    std::unique_ptr<Engine::Registry> registry;
    Engine::Entity root = static_cast<Engine::Entity>(-1);
    std::vector<Snapshot> fileSnapshots;
    std::string absolutePath;
    std::string relativePath;
    bool loadFailed = false;
};

PreviewPanel::PreviewPanel() {
    m_PluginEditors.push_back(std::make_unique<PositionPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<NetworkComponentPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<TextPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<ShapePluginEditor>());
    m_PluginEditors.push_back(std::make_unique<RotationPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<SpritePluginEditor>());
    m_PluginEditors.push_back(std::make_unique<OpacityPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<ColliderPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<CameraPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<ScriptPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<AudioPluginEditor>());
}

PreviewPanel::~PreviewPanel() = default;

void PreviewPanel::EnsurePrefabLoaded(Engine::Registry& scene, EditorConfig& config,
                                      const std::string& absolutePath) {
    if (m_Prefab && m_Prefab->absolutePath == absolutePath && !m_Prefab->loadFailed) {
        return;
    }

    m_Prefab = std::make_unique<PrefabPreviewState>();
    m_Prefab->absolutePath = absolutePath;
    m_Prefab->relativePath = ToProjectRelativePath(scene, absolutePath);
    m_Prefab->registry = std::make_unique<Engine::Registry>();
    RegisterPreviewComponents(*m_Prefab->registry);

    std::string projectFolder = config.m_ProjectFolder;
    if (projectFolder.empty() && scene.HasResource<ProjectContext>()) {
        projectFolder = scene.GetResource<ProjectContext>().projectFolder;
    }
    m_Prefab->registry->AddResource<ProjectContext>(projectFolder);
    m_Prefab->registry->AddResource<EditorConfig>(config);

    if (!EntitySubtree::ReadPrefabFile(absolutePath, m_Prefab->fileSnapshots)) {
        m_Prefab->loadFailed = true;
        log_user_error("Failed to open prefab preview: " + absolutePath);
        return;
    }

    Serialization::GlazeSerializer serializer;
    m_Prefab->root = serializer.SpawnEntities(*m_Prefab->registry, m_Prefab->fileSnapshots);
    if (m_Prefab->root == static_cast<Engine::Entity>(-1)) {
        m_Prefab->loadFailed = true;
        log_error("Failed to open prefab preview: " + absolutePath);
    }
}

void PreviewPanel::DrawPrefabEntityEditors(Engine::Entity entity) {
    Engine::Registry& registry = *m_Prefab->registry;
    ImGui::PushID(static_cast<int>(entity));
    for (auto& editor : m_PluginEditors) {
        editor->OnImGuiRender(entity, registry);
    }
    ImGui::PopID();
}

void PreviewPanel::DrawPrefabChildren(Engine::Entity parent, int level) {
    Engine::Registry& registry = *m_Prefab->registry;
    std::vector<Engine::Entity> children =
        Engine::HierarchySystem::FindDirectChildren(registry, parent);
    std::sort(children.begin(), children.end());
    if (children.empty()) {
        return;
    }

    const std::string parentName = Engine::GetEntityName(registry, parent);
    ImGui::Spacing();
    ImGui::TextDisabled("Children of %s  (level %d)", parentName.c_str(), level);
    ImGui::Separator();

    for (Engine::Entity child : children) {
        ImGui::PushID(static_cast<int>(child));
        const std::string name = Engine::GetEntityName(registry, child);
        // Unframed tree node, not a CollapsingHeader, so a nested entity is not
        // mistaken for a component of its parent (Shape, Text, ...).
        const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen |
                                         ImGuiTreeNodeFlags_OpenOnArrow |
                                         ImGuiTreeNodeFlags_SpanAvailWidth;
        const bool open = ImGui::TreeNodeEx(name.c_str(), flags);
        if (ImGui::BeginPopupContextItem("PrefabChildContext")) {
            DrawAddComponentMenu(m_PluginEditors, child, registry);
            ImGui::EndPopup();
        }
        if (open) {
            DrawPrefabEntityEditors(child);
            DrawPrefabChildren(child, level + 1);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}

void PreviewPanel::DrawPrefabControls() {
    if (!m_Prefab || !m_Prefab->registry || m_Prefab->root == static_cast<Engine::Entity>(-1)) {
        return;
    }

    Engine::Registry& registry = *m_Prefab->registry;
    const Engine::Entity root = m_Prefab->root;
    ImGui::TextUnformatted(Engine::GetEntityName(registry, root).c_str());
    ImGui::Separator();

    DrawPrefabEntityEditors(root);

    if (ImGui::BeginPopupContextWindow("PrefabPreviewContext",
                                       ImGuiPopupFlags_MouseButtonRight |
                                           ImGuiPopupFlags_NoOpenOverItems)) {
        DrawAddComponentMenu(m_PluginEditors, root, registry);
        ImGui::EndPopup();
    }

    DrawPrefabChildren(root, 1);
}

void PreviewPanel::SyncPrefabEdits(Engine::Registry& scene) {
    if (!m_Prefab || m_Prefab->loadFailed || !m_Prefab->registry ||
        m_Prefab->root == static_cast<Engine::Entity>(-1)) {
        return;
    }

    std::vector<Snapshot> current = CaptureSubtree(*m_Prefab->registry, m_Prefab->root);

    // Root position stays in the file so placed instances do not jump. Child
    // positions are kept so offsets edited in the preview are saved. A
    // component removed in the preview is put back here, so instances keep it.
    for (std::size_t i = 0; i < current.size() && i < m_Prefab->fileSnapshots.size(); ++i) {
        if (i == 0) {
            current[i].position = m_Prefab->fileSnapshots[i].position;
        } else {
            KeepIfMissing(current[i].position, m_Prefab->fileSnapshots[i].position);
        }
        KeepIfMissing(current[i].sprite, m_Prefab->fileSnapshots[i].sprite);
        KeepIfMissing(current[i].shape, m_Prefab->fileSnapshots[i].shape);
        KeepIfMissing(current[i].script, m_Prefab->fileSnapshots[i].script);
        KeepIfMissing(current[i].network, m_Prefab->fileSnapshots[i].network);
        KeepIfMissing(current[i].collider, m_Prefab->fileSnapshots[i].collider);
        KeepIfMissing(current[i].text, m_Prefab->fileSnapshots[i].text);
        KeepIfMissing(current[i].opacity, m_Prefab->fileSnapshots[i].opacity);
        KeepIfMissing(current[i].rotation, m_Prefab->fileSnapshots[i].rotation);
        KeepIfMissing(current[i].camera, m_Prefab->fileSnapshots[i].camera);
        KeepIfMissing(current[i].audioSource, m_Prefab->fileSnapshots[i].audioSource);
    }
    if (!current.empty()) {
        current.front().prefab.reset();
    }

    if (!EntitySubtree::WritePrefabFile(m_Prefab->absolutePath, current)) {
        log_error("Failed to write prefab: " + m_Prefab->absolutePath);
        return;
    }

    std::size_t updated = 0;
    if (scene.IsComponentRegistered<Engine::PrefabInstance>()) {
        for (std::size_t id = 1; id < scene.GetEntityCount(); ++id) {
            const Engine::Entity instanceRoot = id;
            if (!scene.IsEntityAlive(instanceRoot) ||
                !scene.HasComponent<Engine::PrefabInstance>(instanceRoot)) {
                continue;
            }
            auto& link = scene.GetComponent<Engine::PrefabInstance>(instanceRoot);
            if (!SamePrefab(scene, link.prefabPath, m_Prefab->relativePath,
                            m_Prefab->absolutePath)) {
                continue;
            }
            link.prefabPath = m_Prefab->relativePath;
            std::vector<Engine::Entity> instanceEntities;
            std::unordered_set<Engine::Entity> seen;
            CollectEntities(scene, instanceRoot, instanceEntities, seen, 0);
            const std::size_t count = std::min(instanceEntities.size(), current.size());
            for (std::size_t i = 0; i < count; ++i) {
                ApplySnapshot(scene, instanceEntities[i], current[i]);
                if (i > 0) {
                    ApplyChildPosition(scene, instanceEntities[i], current, i);
                }
            }
            ++updated;
        }
    }

    m_Prefab->fileSnapshots = std::move(current);
    log_info("Prefab saved: " + m_Prefab->relativePath + " (" + std::to_string(updated) +
             " instances)");
}

void PreviewPanel::RenderPrefab(Engine::Registry& scene, EditorConfig& config,
                                const std::string& absolutePath) {
    m_CurrentPreviewPath = absolutePath;
    if (m_PreviewTexture.IsValid()) {
        m_PreviewTexture.Unload();
    }

    EnsurePrefabLoaded(scene, config, absolutePath);
    const std::string fileName = std::filesystem::path(absolutePath).filename().string();
    const bool canEdit = m_Prefab && !m_Prefab->loadFailed && m_Prefab->registry;

    // Stays outside the scrolling child, so Save remains on screen.
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Previewing: %s", fileName.c_str());
    ImGui::SameLine();
    const float saveWidth = 72.0f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - saveWidth);
    bool save = false;
    if (!canEdit) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Save", ImVec2(saveWidth, 0.0f))) {
        save = true;
    }
    if (!canEdit) {
        ImGui::EndDisabled();
    }
    ImGui::Separator();

    if (!canEdit) {
        ImGui::TextDisabled("Could not read this prefab.");
        return;
    }
    std::unordered_set<Engine::Entity> seen;
    LoadPrefabSprites(*m_Prefab->registry, m_Prefab->root, seen, 0);
    ImGui::BeginChild("##PrefabEditorScroll", ImVec2(0.0f, 0.0f), false);
    DrawPrefabControls();
    ImGui::EndChild();
    if (save) {
        SyncPrefabEdits(scene);
    }
}

void PreviewPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    if (!config.m_SelectedAssetPath.empty()) {
        m_IsOpen = true;
    }

    if (!m_IsOpen) {
        return;
    }

    if (ImGui::Begin(m_Name.c_str())) {
        if (!config.m_SelectedAssetPath.empty()) {
            std::string selectedPath =
                ResolveAssetPath(registry, config.m_SelectedAssetPath).string();

            if (IsPrefabFile(selectedPath)) {
                RenderPrefab(registry, config, selectedPath);
            } else if (m_CurrentPreviewPath != selectedPath) {
                m_CurrentPreviewPath = selectedPath;
                m_Prefab.reset();

                if (IsImageFile(m_CurrentPreviewPath)) {
                    std::string ext =
                        std::filesystem::path(m_CurrentPreviewPath).extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                    try {
                        if (m_PreviewTexture.IsValid()) {
                            m_PreviewTexture.Unload();
                        }
                        if (ext == ".gif") {
                            int animFrames = 0;
                            raylib::Image img;
                            img.Load(m_CurrentPreviewPath, &animFrames);
                            m_PreviewTexture.Load(img);
                        } else {
                            m_PreviewTexture.Load(m_CurrentPreviewPath);
                        }
                    } catch (const raylib::RaylibException&) {
                        m_PreviewTexture.Unload();
                    }
                } else {
                    if (m_PreviewTexture.IsValid()) {
                        m_PreviewTexture.Unload();
                    }
                }
            }

            if (!IsPrefabFile(selectedPath) && m_PreviewTexture.IsValid()) {
                ImGui::Text(
                    "Previewing: %s",
                    std::filesystem::path(m_CurrentPreviewPath).filename().string().c_str());
                ImGui::Separator();

                ImVec2 avail = ImGui::GetContentRegionAvail();
                float aspect = static_cast<float>(m_PreviewTexture.width) /
                               static_cast<float>(m_PreviewTexture.height);
                ImVec2 size;
                if (avail.x / aspect <= avail.y) {
                    size.x = avail.x;
                    size.y = avail.x / aspect;
                } else {
                    size.y = avail.y;
                    size.x = avail.y * aspect;
                }

                ImVec2 cursor = ImGui::GetCursorPos();
                cursor.x += (avail.x - size.x) * 0.5f;
                cursor.y += (avail.y - size.y) * 0.5f;
                ImGui::SetCursorPos(cursor);

                rlImGuiImageSize(&m_PreviewTexture, static_cast<int>(size.x),
                                 static_cast<int>(size.y));
            } else if (!IsPrefabFile(selectedPath)) {
                ImGui::Text(
                    "Previewing: %s",
                    std::filesystem::path(m_CurrentPreviewPath).filename().string().c_str());
                ImGui::TextDisabled("(TODO: support for other files)");
            }
        } else {
            ImGui::Text("No asset selected.");
        }
    }
    ImGui::End();
}

const std::string& PreviewPanel::GetName() const {
    return m_Name;
}
} // namespace Editor
