#include "Panels/PreviewPanel.hpp"
#include "Components/AudioSource.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Input.hpp"
#include "Components/Name.hpp"
#include "Components/Opacity.hpp"
#include "Components/Parent.hpp"
#include "Components/PrefabInstance.hpp"
#include "Components/ShapeComponent.hpp"
#include "Components/SignalConnections.hpp"
#include "Components/SpriteComponent.hpp"
#include "Components/Text.hpp"
#include "EditorConfig.hpp"
#include "EntitySubtree.hpp"
#include "GlazeSerializer.hpp"
#include "Logging/Logger.hpp"
#include "Plugins/CameraPluginEditor.hpp"
#include "Plugins/ColliderPluginEditor.hpp"
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
#include <filesystem>
#include <imgui.h>
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

bool SpriteChanged(const Snapshot& baseline, const Snapshot& current) {
    if (!current.sprite.has_value()) {
        return false;
    }
    if (!baseline.sprite.has_value()) {
        return true;
    }
    return current.sprite->assetPath != baseline.sprite->assetPath ||
           current.sprite->scaleX != baseline.sprite->scaleX ||
           current.sprite->scaleY != baseline.sprite->scaleY;
}

bool ShapeChanged(const Snapshot& baseline, const Snapshot& current) {
    if (!current.shape.has_value()) {
        return false;
    }
    if (!baseline.shape.has_value()) {
        return true;
    }
    return current.shape->width != baseline.shape->width ||
           current.shape->height != baseline.shape->height ||
           current.shape->radius != baseline.shape->radius;
}

bool ScriptChanged(const Snapshot& baseline, const Snapshot& current) {
    if (!current.script.has_value()) {
        return false;
    }
    if (!baseline.script.has_value()) {
        return true;
    }
    return current.script->scriptPath != baseline.script->scriptPath;
}

bool SyncedFieldsChanged(const std::vector<Snapshot>& baseline,
                         const std::vector<Snapshot>& current) {
    const std::size_t count = std::min(baseline.size(), current.size());
    for (std::size_t i = 0; i < count; ++i) {
        if (SpriteChanged(baseline[i], current[i]) || ShapeChanged(baseline[i], current[i]) ||
            ScriptChanged(baseline[i], current[i])) {
            return true;
        }
    }
    if (current.size() > baseline.size()) {
        for (std::size_t i = baseline.size(); i < current.size(); ++i) {
            if (current[i].sprite.has_value() || current[i].shape.has_value() ||
                current[i].script.has_value()) {
                return true;
            }
        }
    }
    return false;
}

void ApplySprite(Engine::Registry& scene, Engine::Entity entity,
                 const Engine::SpriteComponent& source) {
    scene.RegisterComponent<Engine::SpriteComponent>();
    if (!scene.HasComponent<Engine::SpriteComponent>(entity)) {
        scene.AddComponent<Engine::SpriteComponent>(
            entity, Engine::SpriteComponent(nullptr, "", 0.0f, 0.0f, source.scaleX, source.scaleY));
    }
    auto& sprite = scene.GetComponent<Engine::SpriteComponent>(entity);
    if (sprite.assetPath != source.assetPath) {
        LoadSpriteForComponent(scene, sprite, source.assetPath);
    }
    sprite.scaleX = source.scaleX;
    sprite.scaleY = source.scaleY;
    if (sprite.sprite) {
        sprite.sprite->SetScale({sprite.scaleX, sprite.scaleY});
    }
}

void ApplyShape(Engine::Registry& scene, Engine::Entity entity, const Engine::ShapeComponent& source) {
    scene.RegisterComponent<Engine::ShapeComponent>();
    if (!scene.HasComponent<Engine::ShapeComponent>(entity)) {
        scene.AddComponent<Engine::ShapeComponent>(entity, Engine::ShapeComponent{});
    }
    auto& shape = scene.GetComponent<Engine::ShapeComponent>(entity);
    shape.width = source.width;
    shape.height = source.height;
    shape.radius = source.radius;
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
        ApplyShape(scene, entity, *source.shape);
    }
    if (source.script.has_value()) {
        ApplyScript(scene, entity, *source.script);
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
    m_PluginEditors.push_back(std::make_unique<TextPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<ShapePluginEditor>());
    m_PluginEditors.push_back(std::make_unique<RotationPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<SpritePluginEditor>());
    m_PluginEditors.push_back(std::make_unique<OpacityPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<ColliderPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<CameraPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<ScriptPluginEditor>());
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
        log_error("Failed to open prefab preview: " + absolutePath);
        return;
    }

    Serialization::GlazeSerializer serializer;
    m_Prefab->root = serializer.SpawnEntities(*m_Prefab->registry, m_Prefab->fileSnapshots);
    if (m_Prefab->root == static_cast<Engine::Entity>(-1)) {
        m_Prefab->loadFailed = true;
        log_error("Failed to open prefab preview: " + absolutePath);
    }
}

void PreviewPanel::DrawPrefabControls() {
    if (!m_Prefab || !m_Prefab->registry ||
        m_Prefab->root == static_cast<Engine::Entity>(-1)) {
        return;
    }

    Engine::Registry& registry = *m_Prefab->registry;
    const Engine::Entity root = m_Prefab->root;
    ImGui::TextUnformatted(Engine::GetEntityName(registry, root).c_str());
    ImGui::Separator();

    for (auto& editor : m_PluginEditors) {
        editor->OnImGuiRender(root, registry);
    }

    if (ImGui::BeginPopupContextWindow("PrefabPreviewContext",
                                       ImGuiPopupFlags_MouseButtonRight |
                                           ImGuiPopupFlags_NoOpenOverItems)) {
        bool anyAvailable = false;
        for (auto& editor : m_PluginEditors) {
            if (!editor->HasComponent(root, registry)) {
                anyAvailable = true;
                const std::string menuItem =
                    std::string(editor->GetIcon()) + " Add " + editor->GetName();
                if (ImGui::MenuItem(menuItem.c_str())) {
                    editor->AddComponent(root, registry);
                }
            }
        }
        if (!anyAvailable) {
            ImGui::TextDisabled("All plugins added");
        }
        ImGui::EndPopup();
    }
}

void PreviewPanel::SyncPrefabEdits(Engine::Registry& scene) {
    if (!m_Prefab || m_Prefab->loadFailed || !m_Prefab->registry ||
        m_Prefab->root == static_cast<Engine::Entity>(-1)) {
        return;
    }

    std::vector<Snapshot> current = CaptureSubtree(*m_Prefab->registry, m_Prefab->root);
    if (!SyncedFieldsChanged(m_Prefab->fileSnapshots, current)) {
        return;
    }

    // Positions stay in the file so placed instances do not jump. A component
    // removed in the preview is put back here, so instances keep it too.
    for (std::size_t i = 0; i < current.size() && i < m_Prefab->fileSnapshots.size(); ++i) {
        current[i].position = m_Prefab->fileSnapshots[i].position;
        if (!current[i].sprite.has_value()) {
            current[i].sprite = m_Prefab->fileSnapshots[i].sprite;
        }
        if (!current[i].shape.has_value()) {
            current[i].shape = m_Prefab->fileSnapshots[i].shape;
        }
        if (!current[i].script.has_value()) {
            current[i].script = m_Prefab->fileSnapshots[i].script;
        }
    }
    if (!current.empty()) {
        current.front().prefab.reset();
    }

    if (!EntitySubtree::WritePrefabFile(m_Prefab->absolutePath, current)) {
        log_error("Failed to write prefab: " + m_Prefab->absolutePath);
        return;
    }

    if (scene.IsComponentRegistered<Engine::PrefabInstance>()) {
        for (std::size_t id = 1; id < scene.GetEntityCount(); ++id) {
            const Engine::Entity instanceRoot = id;
            if (!scene.IsEntityAlive(instanceRoot) ||
                !scene.HasComponent<Engine::PrefabInstance>(instanceRoot)) {
                continue;
            }
            if (scene.GetComponent<Engine::PrefabInstance>(instanceRoot).prefabPath !=
                m_Prefab->relativePath) {
                continue;
            }
            std::vector<Engine::Entity> instanceEntities;
            std::unordered_set<Engine::Entity> seen;
            CollectEntities(scene, instanceRoot, instanceEntities, seen, 0);
            const std::size_t count = std::min(instanceEntities.size(), current.size());
            for (std::size_t i = 0; i < count; ++i) {
                ApplySnapshot(scene, instanceEntities[i], current[i]);
            }
        }
    }

    m_Prefab->fileSnapshots = std::move(current);
}

void PreviewPanel::RenderPrefab(Engine::Registry& scene, EditorConfig& config,
                                const std::string& absolutePath) {
    m_CurrentPreviewPath = absolutePath;
    if (m_PreviewTexture.IsValid()) {
        m_PreviewTexture.Unload();
    }

    EnsurePrefabLoaded(scene, config, absolutePath);
    ImGui::Text("Previewing: %s",
                std::filesystem::path(absolutePath).filename().string().c_str());
    ImGui::Separator();
    if (!m_Prefab || m_Prefab->loadFailed || !m_Prefab->registry) {
        ImGui::TextDisabled("Could not read this prefab.");
        return;
    }
    std::unordered_set<Engine::Entity> seen;
    LoadPrefabSprites(*m_Prefab->registry, m_Prefab->root, seen, 0);
    DrawPrefabControls();
    SyncPrefabEdits(scene);
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
