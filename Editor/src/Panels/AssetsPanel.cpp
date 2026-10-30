#include "Panels/AssetsPanel.hpp"
#include "Components/AudioSource.hpp"
#include "Components/PrefabInstance.hpp"
#include "Components/SpriteComponent.hpp"
#include "EditorConfig.hpp"
#include "EditorLayer.hpp"
#include "EntitySubtree.hpp"
#include "Logging/Logger.hpp"
#include "ScriptEditorService.hpp"
#include "Scripting/ScriptComponent.hpp"
#include <algorithm>
#include <cctype>
#include <extras/IconsFontAwesome6.h>
#include <fstream>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <optional>

namespace Editor {
namespace {
bool IsInsideDirectory(const std::filesystem::path& directory, const std::filesystem::path& path) {
    std::error_code error;
    const std::filesystem::path relative = std::filesystem::relative(path, directory, error);
    if (error) {
        return false;
    }
    const std::string text = relative.generic_string();
    return !text.empty() && text != ".." && text.rfind("../", 0) != 0;
}

bool IsLuaPath(const std::filesystem::path& path) {
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".lua";
}

void RequestOpenScript(Engine::Registry& registry, const std::filesystem::path& path) {
    if (!IsLuaPath(path) || !registry.HasResource<ScriptEditorService>()) {
        return;
    }
    registry.GetResource<ScriptEditorService>().Open(path);
}

bool IsValidEntryName(const std::string& name) {
    if (name.empty() || name == "." || name == "..") {
        return false;
    }
    return name.find('/') == std::string::npos && name.find('\\') == std::string::npos;
}

std::string ProjectRelative(const std::filesystem::path& project,
                            const std::filesystem::path& path) {
    std::error_code error;
    const std::filesystem::path relative = std::filesystem::relative(path, project, error);
    if (error) {
        return "";
    }
    return relative.generic_string();
}

bool RetargetStoredPath(std::string& stored, const std::string& from, const std::string& to) {
    if (from.empty() || stored.empty()) {
        return false;
    }
    if (stored == from) {
        stored = to;
        return true;
    }
    const std::string prefix = from + "/";
    if (stored.rfind(prefix, 0) == 0) {
        stored = to + stored.substr(from.size());
        return true;
    }
    return false;
}

void ReplaceAll(std::string& text, const std::string& from, const std::string& to) {
    if (from.empty() || from == to) {
        return;
    }
    std::size_t pos = 0;
    while ((pos = text.find(from, pos)) != std::string::npos) {
        text.replace(pos, from.size(), to);
        pos += to.size();
    }
}

void RetargetRegistry(Engine::Registry& registry, const std::string& from, const std::string& to) {
    for (std::size_t id = 0; id < registry.GetEntityCount(); ++id) {
        const Engine::Entity entity = id;
        if (!registry.IsEntityAlive(entity)) {
            continue;
        }
        if (registry.IsComponentRegistered<Engine::SpriteComponent>() &&
            registry.HasComponent<Engine::SpriteComponent>(entity)) {
            RetargetStoredPath(registry.RequireComponent<Engine::SpriteComponent>(entity).assetPath,
                               from, to);
        }
        if (registry.IsComponentRegistered<Engine::ScriptComponent>() &&
            registry.HasComponent<Engine::ScriptComponent>(entity)) {
            auto& script = registry.RequireComponent<Engine::ScriptComponent>(entity);
            if (RetargetStoredPath(script.scriptPath, from, to)) {
                script.loaded = false;
                script.scriptTable = sol::lua_nil;
            }
        }
        if (registry.IsComponentRegistered<Engine::PrefabInstance>() &&
            registry.HasComponent<Engine::PrefabInstance>(entity)) {
            RetargetStoredPath(registry.RequireComponent<Engine::PrefabInstance>(entity).prefabPath,
                               from, to);
        }
        if (registry.IsComponentRegistered<Engine::AudioSource>() &&
            registry.HasComponent<Engine::AudioSource>(entity)) {
            RetargetStoredPath(registry.RequireComponent<Engine::AudioSource>(entity).assetId, from,
                               to);
        }
    }
}

void RetargetSavedFiles(const std::filesystem::path& project, const std::string& from,
                        const std::string& to) {
    const std::string fromExact = "\"" + from + "\"";
    const std::string toExact = "\"" + to + "\"";
    const std::string fromPrefix = "\"" + from + "/";
    const std::string toPrefix = "\"" + to + "/";
    std::error_code error;
    const auto options = std::filesystem::directory_options::skip_permission_denied;
    for (std::filesystem::recursive_directory_iterator it(project, options, error), end;
         !error && it != end; it.increment(error)) {
        if (error || !it->is_regular_file(error)) {
            error.clear();
            continue;
        }
        std::string extension = it->path().extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (extension != ".prefab" && extension != ".scene") {
            continue;
        }
        std::ifstream input(it->path());
        if (!input.is_open()) {
            continue;
        }
        std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        input.close();
        if (text.find(fromExact) == std::string::npos &&
            text.find(fromPrefix) == std::string::npos) {
            continue;
        }
        const std::string original = text;
        ReplaceAll(text, fromPrefix, toPrefix);
        ReplaceAll(text, fromExact, toExact);
        if (text == original) {
            continue;
        }
        std::ofstream output(it->path(), std::ios::trunc);
        if (!output.is_open()) {
            log_error("Assets: failed to update paths in " + it->path().string());
            continue;
        }
        output << text;
    }
}

bool IsSameFile(const std::filesystem::path& a, const std::string& b) {
    if (b.empty()) {
        return false;
    }
    std::error_code error;
    return std::filesystem::weakly_canonical(a, error) ==
           std::filesystem::weakly_canonical(std::filesystem::path(b), error);
}

void FollowMovedPath(std::filesystem::path& path, const std::filesystem::path& from,
                     const std::filesystem::path& to) {
    if (path.empty()) {
        return;
    }
    std::error_code error;
    const std::filesystem::path relative = std::filesystem::relative(path, from, error);
    if (error) {
        return;
    }
    const std::string text = relative.generic_string();
    if (text == ".") {
        path = to;
        return;
    }
    if (text.empty() || text == ".." || text.rfind("../", 0) == 0) {
        return;
    }
    path = to / relative;
}
} // namespace

void AssetsPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& cfg) {
    if (!ImGui::Begin(m_Name.c_str())) {
        ImGui::End();
        return;
    }

    if (cfg.m_ProjectFolder.empty()) {
        ImGui::Text("No project loaded.");
        ImGui::End();
        return;
    }

    std::error_code error;

    std::filesystem::path projectPath =
        std::filesystem::weakly_canonical(std::filesystem::path(cfg.m_ProjectFolder), error);

    if (error || !std::filesystem::exists(projectPath) ||
        !std::filesystem::is_directory(projectPath)) {
        ImGui::Text("Invalid project folder.");
        ImGui::End();
        return;
    }

    /*
     * Reset the navigation root when a different project is loaded.
     *
     * m_StartingDirectory is the highest directory the user is allowed
     * to navigate back to.
     */
    if (m_LastProjectDirectory != projectPath) {
        m_LastProjectDirectory = projectPath;
        m_StartingDirectory = projectPath;
        m_CurrentDirectory = projectPath;
    }

    if (m_StartingDirectory.empty()) {
        m_StartingDirectory = projectPath;
    }

    if (m_CurrentDirectory.empty() || !std::filesystem::exists(m_CurrentDirectory) ||
        !std::filesystem::is_directory(m_CurrentDirectory)) {
        m_CurrentDirectory = m_StartingDirectory;
    }

    /*
     * Right-click context menu for the current folder.
     */
    if (ImGui::BeginPopupContextWindow("AssetsFolderContext",
                                       ImGuiPopupFlags_MouseButtonRight |
                                           ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::MenuItem("New Folder")) {
            m_DialogText.clear();
            m_OpenNewFolder = true;
        }
        if (ImGui::MenuItem("New Scene...")) {
            cfg.m_RequestedNewSceneFolder = m_CurrentDirectory.string();
        }
        if (ImGui::Button("Refresh Folder")) {
            /*
             * The folder contents are enumerated directly below every
             * frame, so normalizing the path is enough to refresh it.
             */
            if (std::filesystem::exists(m_CurrentDirectory) &&
                std::filesystem::is_directory(m_CurrentDirectory)) {
                m_CurrentDirectory = std::filesystem::weakly_canonical(m_CurrentDirectory, error);

                if (error) {
                    m_CurrentDirectory = m_StartingDirectory;
                    error.clear();
                }
            } else {
                m_CurrentDirectory = m_StartingDirectory;
            }

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    /*
     * Only allow going back if we are below the starting directory.
     */
    const bool canGoBack = !m_CurrentDirectory.empty() && m_CurrentDirectory != m_StartingDirectory;

    if (canGoBack) {
        if (ImGui::Button("<- Back")) {
            const std::filesystem::path parentDirectory = m_CurrentDirectory.parent_path();

            /*
             * Prevent invalid navigation at the filesystem root.
             * Because folders are only entered from this function,
             * this will never move above m_StartingDirectory.
             */
            if (!parentDirectory.empty() && parentDirectory != m_CurrentDirectory) {
                m_CurrentDirectory = parentDirectory;
            }
        }

        ImGui::SameLine();
    }

    ImGui::Text("%s", m_CurrentDirectory.string().c_str());

    ImGui::Separator();

    constexpr float padding = 16.0f;
    constexpr float thumbnailSize = 64.0f;
    constexpr float cellSize = thumbnailSize + padding;

    const float panelWidth = ImGui::GetContentRegionAvail().x;

    int columnCount = static_cast<int>(panelWidth / cellSize);

    if (columnCount < 1) {
        columnCount = 1;
    }

    std::optional<std::filesystem::path> pendingSource;
    std::optional<std::filesystem::path> pendingDestination;

    if (ImGui::BeginTable("AssetsTable", columnCount)) {
        std::filesystem::directory_iterator directoryIterator(m_CurrentDirectory, error);

        if (!error) {
            for (const auto& directoryEntry : directoryIterator) {
                const std::filesystem::path path = directoryEntry.path();

                const std::string filenameString = path.filename().string();

                ImGui::TableNextColumn();
                ImGui::PushID(filenameString.c_str());

                if (directoryEntry.is_directory()) {
                    ImGui::PushFont(g_IconFont);
                    bool clicked =
                        ImGui::Button(ICON_FA_FOLDER, ImVec2(thumbnailSize, thumbnailSize));
                    ImGui::PopFont();

                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        const std::string relativePath =
                            std::filesystem::relative(path, m_StartingDirectory).string();
                        ImGui::SetDragDropPayload("ASSET_MOVE", relativePath.c_str(),
                                                  relativePath.size() + 1);
                        ImGui::Text("%s", filenameString.c_str());
                        ImGui::EndDragDropSource();
                    }
                    if (ImGui::BeginDragDropTarget()) {
                        const ImGuiPayload* movePayload =
                            ImGui::AcceptDragDropPayload("ASSET_MOVE");
                        if (movePayload == nullptr) {
                            movePayload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM");
                        }
                        if (movePayload != nullptr && movePayload->Data != nullptr) {
                            const std::string relativePath(
                                static_cast<const char*>(movePayload->Data));
                            const std::filesystem::path source = m_StartingDirectory / relativePath;
                            pendingSource = source;
                            pendingDestination = path / source.filename();
                        }
                        if (const ImGuiPayload* entityPayload =
                                ImGui::AcceptDragDropPayload("ENTITY_ID")) {
                            const Engine::Entity dragged =
                                *static_cast<const Engine::Entity*>(entityPayload->Data);
                            EntitySubtree::SavePrefab(registry, dragged, path);
                        }
                        ImGui::EndDragDropTarget();
                    }

                    if (clicked) {
                        /*
                         * Enter the folder and normalize its path.
                         */
                        m_CurrentDirectory = std::filesystem::weakly_canonical(path, error);

                        if (error) {
                            m_CurrentDirectory = m_StartingDirectory;
                            error.clear();
                        }
                    }
                } else {
                    const bool isScene = path.extension() == ".scene";
                    ImGui::PushFont(g_IconFont);
                    if (ImGui::Button(isScene ? ICON_FA_MAP : ICON_FA_FILE,
                                      ImVec2(thumbnailSize, thumbnailSize))) {
                        cfg.m_SelectedAssetPath = path.string();
                    }
                    if (ImGui::IsItemClicked()) {
                        cfg.m_SelectedAssetPath = path.string();
                    }
                    if (ImGui::IsItemHovered() &&
                        ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        RequestOpenScript(registry, path);
                    }
                    ImGui::PopFont();
                    // Double-click a scene to open it (EditorLayer asks to save first).
                    if (isScene && ImGui::IsItemHovered() &&
                        ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        cfg.m_RequestedScenePath = path.string();
                    }
                    if (isScene && ImGui::IsItemHovered()) {
                        ImGui::SetTooltip(IsSameFile(path, cfg.m_InitialScenePath)
                                              ? "Startup scene: the game starts here.\n"
                                                "Double-click to open it."
                                              : "Double-click to open this scene");
                    }
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        cfg.m_SelectedAssetPath = path.string();
                        std::string relativePath =
                            std::filesystem::relative(path, m_StartingDirectory).string();
                        ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", relativePath.c_str(),
                                                  relativePath.size() + 1);
                        ImGui::Text("%s", filenameString.c_str());
                        ImGui::EndDragDropSource();
                    }
                }

                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    m_DialogTarget = path;
                    ImGui::OpenPopup("AssetItemMenu");
                }

                const bool isStartupScene = !directoryEntry.is_directory() &&
                                            IsSameFile(path, cfg.m_InitialScenePath);
                if (isStartupScene) {
                    ImGui::TextWrapped(ICON_FA_FLAG " %s", filenameString.c_str());
                } else {
                    ImGui::TextWrapped("%s", filenameString.c_str());
                }
                if (ImGui::IsItemClicked()) {
                    cfg.m_SelectedAssetPath = path.string();
                }
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    RequestOpenScript(registry, path);
                }
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    m_DialogTarget = path;
                    ImGui::OpenPopup("AssetItemMenu");
                }

                ImGui::PopID();
            }
        }

        ImGui::EndTable();
    }

    if (pendingSource.has_value() && pendingDestination.has_value()) {
        Relocate(registry, cfg, *pendingSource, *pendingDestination);
    }

    // File buttons do not cover the gaps, so the drop target is the panel rect.
    // BeginDragDropTarget only attaches to the last submitted item.
    const ImVec2 regionMin = ImGui::GetWindowContentRegionMin();
    const ImVec2 regionMax = ImGui::GetWindowContentRegionMax();
    const ImVec2 windowPos = ImGui::GetWindowPos();
    const ImRect dropRect(ImVec2(windowPos.x + regionMin.x, windowPos.y + regionMin.y),
                          ImVec2(windowPos.x + regionMax.x, windowPos.y + regionMax.y));
    if (ImGui::BeginDragDropTargetCustom(dropRect, ImGui::GetID("##entity-prefab-drop"))) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_ID")) {
            const Engine::Entity dragged = *static_cast<const Engine::Entity*>(payload->Data);
            EntitySubtree::SavePrefab(registry, dragged, m_CurrentDirectory);
        }
        ImGui::EndDragDropTarget();
    }

    DrawDialogs(registry, cfg);

    ImGui::End();
}

} // namespace Editor

namespace Editor {
void AssetsPanel::DrawDialogs(Engine::Registry& registry, EditorConfig& config) {
    if (m_OpenNewFolder) {
        ImGui::OpenPopup("NewFolder");
        m_OpenNewFolder = false;
    }
    if (ImGui::BeginPopup("AssetItemMenu")) {
        bool openRename = false;
        bool openDelete = false;
        if (m_DialogTarget.extension() == ".scene") {
            if (ImGui::MenuItem("Open Scene")) {
                config.m_RequestedScenePath = m_DialogTarget.string();
            }
            const bool isStartup = IsSameFile(m_DialogTarget, config.m_InitialScenePath);
            if (ImGui::MenuItem("Set as Startup Scene", nullptr, isStartup, !isStartup)) {
                config.m_RequestedStartupScene = m_DialogTarget.string();
            }
            ImGui::Separator();
        }
        if (ImGui::MenuItem("Rename")) {
            m_DialogText = m_DialogTarget.filename().string();
            openRename = true;
        }
        if (ImGui::MenuItem("Delete")) {
            openDelete = true;
        }
        if (IsLuaPath(m_DialogTarget) && ImGui::MenuItem("Open in Script Editor")) {
            RequestOpenScript(registry, m_DialogTarget);
        }
        ImGui::EndPopup();
        if (openRename) {
            ImGui::OpenPopup("RenameAsset");
        }
        if (openDelete) {
            ImGui::OpenPopup("DeleteAsset");
        }
    }

    if (ImGui::BeginPopupModal("NewFolder", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Name", &m_DialogText);
        if (ImGui::Button("Create")) {
            if (!IsValidEntryName(m_DialogText)) {
                log_user_error("Assets: invalid folder name");
            } else {
                const std::filesystem::path folder = m_CurrentDirectory / m_DialogText;
                if (!IsInsideDirectory(m_StartingDirectory, folder) &&
                    folder != m_StartingDirectory) {
                    log_user_error("Assets: folder must stay inside the project");
                } else if (std::filesystem::exists(folder)) {
                    log_user_error("Assets: " + folder.string() + " already exists");
                } else {
                    std::error_code error;
                    std::filesystem::create_directory(folder, error);
                    if (error) {
                        log_error("Assets: failed to create " + folder.string());
                    } else {
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("RenameAsset", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Name", &m_DialogText);
        if (ImGui::Button("Rename")) {
            if (!IsValidEntryName(m_DialogText)) {
                log_user_error("Assets: invalid name");
            } else if (Relocate(registry, config, m_DialogTarget,
                                m_DialogTarget.parent_path() / m_DialogText)) {
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("DeleteAsset", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Delete %s?", m_DialogTarget.filename().string().c_str());
        if (ImGui::Button("Delete")) {
            if (m_DialogTarget == m_StartingDirectory) {
                log_user_error("Assets: the project folder cannot be deleted");
            } else if (!IsInsideDirectory(m_StartingDirectory, m_DialogTarget)) {
                log_user_error("Assets: cannot delete a path outside the project");
            } else {
                std::error_code error;
                std::filesystem::remove_all(m_DialogTarget, error);
                if (error) {
                    log_error("Assets: failed to delete " + m_DialogTarget.string());
                } else {
                    const std::filesystem::path selected(config.m_SelectedAssetPath);
                    if (selected == m_DialogTarget || IsInsideDirectory(m_DialogTarget, selected)) {
                        config.m_SelectedAssetPath.clear();
                    }
                    if (m_CurrentDirectory == m_DialogTarget ||
                        IsInsideDirectory(m_DialogTarget, m_CurrentDirectory)) {
                        m_CurrentDirectory = m_StartingDirectory;
                    }
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

bool AssetsPanel::Relocate(Engine::Registry& registry, EditorConfig& config,
                           const std::filesystem::path& source,
                           const std::filesystem::path& destination) {
    if (source == destination) {
        return true;
    }
    if (source == m_StartingDirectory || !IsInsideDirectory(m_StartingDirectory, source)) {
        log_user_error("Assets: cannot move the project root");
        return false;
    }
    if (!IsInsideDirectory(m_StartingDirectory, destination) &&
        destination != m_StartingDirectory) {
        log_user_error("Assets: destination must stay inside the project");
        return false;
    }
    if (std::filesystem::is_directory(source) &&
        (destination == source || IsInsideDirectory(source, destination))) {
        log_user_error("Assets: cannot move a folder into itself");
        return false;
    }
    if (std::filesystem::exists(destination)) {
        log_user_error("Assets: " + destination.string() + " already exists");
        return false;
    }

    const std::string from = ProjectRelative(m_StartingDirectory, source);
    const std::string to = ProjectRelative(m_StartingDirectory, destination);
    if (from.empty() || to.empty() || from == "." || to == ".") {
        log_error("Assets: failed to resolve the project path");
        return false;
    }

    std::error_code error;
    std::filesystem::rename(source, destination, error);
    if (error) {
        log_error("Assets: failed to move " + source.string());
        return false;
    }

    RetargetRegistry(registry, from, to);
    RetargetSavedFiles(m_StartingDirectory, from, to);

    std::filesystem::path selected(config.m_SelectedAssetPath);
    FollowMovedPath(selected, source, destination);
    config.m_SelectedAssetPath = selected.string();
    FollowMovedPath(m_CurrentDirectory, source, destination);
    // Keep the open scene and the startup scene pointing at their files.
    // Canonical paths, so /tmp vs /private/tmp style aliases still match.
    for (std::string* scenePath : {&config.m_ActiveScenePath, &config.m_InitialScenePath}) {
        if (scenePath->empty()) {
            continue;
        }
        std::filesystem::path followed = std::filesystem::weakly_canonical(*scenePath, error);
        error.clear();
        const std::filesystem::path before = followed;
        FollowMovedPath(followed, source, destination);
        if (followed != before) {
            *scenePath = followed.string();
            if (scenePath == &config.m_InitialScenePath) {
                // EditorLayer rewrites project.json with the new location.
                config.m_RequestedStartupScene = followed.string();
            }
        }
    }
    log_info("Assets: moved " + from + " to " + to);
    return true;
}

const std::string& AssetsPanel::GetName() const {
    return m_Name;
}
} // namespace Editor
