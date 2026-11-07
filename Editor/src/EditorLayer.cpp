#include "EditorLayer.hpp"
#include "EditorStyle.hpp"
#include "Gallery.hpp"
#include "Logging/Logger.hpp"
#include "Panels/AssetsPanel.hpp"
#include "Panels/ChatPanel.hpp"
#include "Panels/ConnectionsPanel.hpp"
#include "Panels/ConsolePanel.hpp"
#include "Panels/HierarchyPanel.hpp"
#include "Panels/InputMappingPanel.hpp"
#include "Panels/InspectorPanel.hpp"
#include "Panels/PreviewPanel.hpp"
#include "Panels/SceneViewPanel.hpp"
#include "Panels/ScriptEditorPanel.hpp"
#include "Panels/SettingsPanel.hpp"
#include "ProjectFile.hpp"
#include "Resources/PlayerSlotState.hpp"
#include "SceneSerializer.hpp"
#include "ScriptEditorService.hpp"
#include <algorithm>
#include <cstdlib>
#include <extras/FA6FreeSolidFontData.h>
#include <extras/IconsFontAwesome6.h>
#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <rlImGui.h>

#ifdef _WIN32
#include "Compat/WindowsApi.hpp"
#else
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "BundledAssets.hpp"
#include "Panels/StatePanel.hpp"
#include "Panels/UiPalettePanel.hpp"
#include "Panels/WavePanel.hpp"

namespace Editor {
ImFont* g_IconFont = nullptr;
ImFont* g_CodeFont = nullptr;

EditorLayer::EditorLayer() {
    AddPanel(std::make_unique<HierarchyPanel>());
    AddPanel(std::make_unique<InspectorPanel>());
    AddPanel(std::make_unique<ConnectionsPanel>());
    AddPanel(std::make_unique<ChatPanel>());
    AddPanel(std::make_unique<ConsolePanel>());
    AddPanel(std::make_unique<AssetsPanel>());
    AddPanel(std::make_unique<SceneViewPanel>());
    AddPanel(std::make_unique<PreviewPanel>());
    AddPanel(std::make_unique<SettingsPanel>());
    AddPanel(std::make_unique<InputMappingPanel>());
    AddPanel(std::make_unique<StatePanel>());
    AddPanel(std::make_unique<WavePanel>());
    AddPanel(std::make_unique<UiPalettePanel>());
    auto scriptEditor = std::make_unique<ScriptEditorPanel>();
    m_ScriptEditorPanel = scriptEditor.get();
    AddPanel(std::move(scriptEditor));
}

EditorLayer::~EditorLayer() {
    Shutdown();
}

void EditorLayer::Init() {
    m_Registry->AddResource<HierarchyPanelSelectedEntity>();
    m_Registry->AddResource<StatePanelSelectedState>();
    if (!m_Registry->HasResource<ScriptEditorService>()) {
        m_Registry->AddResource<ScriptEditorService>();
    }
    if (!m_Registry->HasResource<ProjectContext>()) {
        m_Registry->AddResource<ProjectContext>(m_Configuration ? m_Configuration->m_ProjectFolder
                                                                : "");
    } else {
        m_Registry->GetResource<ProjectContext>().projectFolder =
            m_Configuration ? m_Configuration->m_ProjectFolder : "";
    }
    // Set a callback to load custom fonts instead of the default rlImGui font
    rlImGuiSetLoadFontsCallback([]() {
        ImGuiIO& io = ImGui::GetIO();

        ImFontConfig font_config;
        font_config.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromMemoryTTF(
            const_cast<unsigned char*>(BundledAssets::fonts_Inter_Regular_ttf),
            BundledAssets::fonts_Inter_Regular_ttf_size, 16.0f, &font_config);

        static constexpr ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
        ImFontConfig icons_config;
        icons_config.MergeMode = true;
        icons_config.PixelSnapH = true;
        icons_config.FontDataOwnedByAtlas = false;

        io.Fonts->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data,
                                                 fa_solid_900_compressed_size, 16.0f, &icons_config,
                                                 icons_ranges);

        ImFontConfig code_config;
        code_config.FontDataOwnedByAtlas = false;
        code_config.OversampleH = 2;
        code_config.OversampleV = 2;
        g_CodeFont = io.Fonts->AddFontFromMemoryTTF(
            const_cast<unsigned char*>(BundledAssets::fonts_NotoSansMono_Regular_ttf),
            BundledAssets::fonts_NotoSansMono_Regular_ttf_size, 15.0f, &code_config);

        ImFontConfig icons_config_large;
        icons_config_large.MergeMode = false;
        icons_config_large.PixelSnapH = true;
        icons_config_large.FontDataOwnedByAtlas = false;
        g_IconFont = io.Fonts->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data,
                                                              fa_solid_900_compressed_size, 48.0f,
                                                              &icons_config_large, icons_ranges);
    });

    rlImGuiBeginInitImGui();

    EditorStyle::Apply();

    rlImGuiEndInitImGui();

    Serialization::SceneSerializer serializer;
    serializer.Deserialize(m_Configuration->m_ActiveScenePath, *m_Registry, *m_Configuration);
    MarkSceneSaved();

    // Enable docking feature
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void EditorLayer::Shutdown() {
    m_Panels.clear();
    rlImGuiShutdown();
}

void EditorLayer::AddPanel(std::unique_ptr<IEditorPanel> panel) {
    m_Panels.push_back(std::move(panel));
}

void EditorLayer::OnUpdate() {
    rlImGuiBegin();

    // Before any panel draws: a scene switch must not happen mid-iteration.
    HandleSceneRequests();
    // Build output is collected on the build thread; log it here.
    SettingsPanel::ReportBuildResult();

    BeginDockSpace();

    // Route selected entity from Hierarchy to Inspector
    HierarchyPanel* hierarchy = nullptr;
    InspectorPanel* inspector = nullptr;

    for (auto& panel : m_Panels) {
        if (const auto h = dynamic_cast<HierarchyPanel*>(panel.get())) {
            hierarchy = h;
        }
        if (const auto i = dynamic_cast<InspectorPanel*>(panel.get())) {
            inspector = i;
        }
    }

    // if (hierarchy && inspector) {
    //     inspector->SetSelectedEntity(hierarchy->GetSelectedEntity());
    // }

    // Render all registered panels
    for (auto& panel : m_Panels) {
        if (m_Registry && m_Configuration) {
            panel->OnImGuiRender(*m_Registry, *m_Configuration);
        }
    }
    static bool s_FirstFrame = true;
    if (s_FirstFrame) {
        ImGui::SetWindowFocus("Hierarchy");
        s_FirstFrame = false;
    }
    EndDockSpace();

    rlImGuiEnd();
}

namespace fs = std::filesystem;

static fs::path ResolveProjectPath(const fs::path& projectFolder, const fs::path& filePath) {
    if (filePath.is_absolute()) {
        return filePath.lexically_normal();
    }

    return (projectFolder / filePath).lexically_normal();
}

bool EditorLayer::SaveActiveScene() {
    if (!m_Registry || !m_Configuration || m_Configuration->m_ActiveScenePath.empty()) {
        return false;
    }
    const fs::path scenePath =
        ResolveProjectPath(m_Configuration->m_ProjectFolder, m_Configuration->m_ActiveScenePath);
    std::error_code error;
    fs::create_directories(scenePath.parent_path(), error);
    Serialization::SceneSerializer serializer;
    serializer.Serialize(scenePath.string(), *m_Registry, *m_Configuration);
    MarkSceneSaved();
    return true;
}

std::string EditorLayer::ActiveSceneRelativePath() const {
    if (!m_Configuration || m_Configuration->m_ActiveScenePath.empty()) {
        return "";
    }
    const fs::path scenePath =
        ResolveProjectPath(m_Configuration->m_ProjectFolder, m_Configuration->m_ActiveScenePath);
    std::error_code error;
    const fs::path relative =
        fs::relative(scenePath, fs::absolute(m_Configuration->m_ProjectFolder, error), error);
    const std::string text = relative.generic_string();
    if (error || text.empty() || text.rfind("..", 0) == 0) {
        // Outside the project folder: the game gets the absolute path.
        return scenePath.generic_string();
    }
    return text;
}

namespace {
// Frames to wait after a load/save before recording the "saved" state, so
// editor systems have settled derived values (prevX/prevY, child positions).
constexpr int kSnapshotSettleFrames = 3;
int s_SnapshotFramesLeft = 0;

bool SamePath(const fs::path& a, const fs::path& b) {
    std::error_code error;
    return fs::weakly_canonical(a, error) == fs::weakly_canonical(b, error);
}

// Folders never scanned for scenes: build output and hidden folders.
bool IsSkippedFolder(const fs::path& folder) {
    const std::string name = folder.filename().string();
    return name.empty() || name[0] == '.' || name == "build" || name.rfind("cmake-build", 0) == 0;
}
} // namespace

std::string EditorLayer::CaptureSceneJson() const {
    if (!m_Registry) {
        return "";
    }
    Serialization::GlazeSerializer serializer;
    return serializer.SerializeToString(serializer.CaptureScene(*m_Registry));
}

void EditorLayer::MarkSceneSaved() {
    m_SavedSceneJsonStale = true;
    s_SnapshotFramesLeft = kSnapshotSettleFrames;
}

bool EditorLayer::HasUnsavedChanges() {
    if (m_SavedSceneJsonStale) {
        // Saved or loaded a moment ago: nothing can have changed yet.
        return false;
    }
    return CaptureSceneJson() != m_SavedSceneJson;
}

void EditorLayer::ClearScene() const {
    m_Registry->ClearEntities();
    if (m_Registry->HasResource<HierarchyPanelSelectedEntity>()) {
        m_Registry->GetResource<HierarchyPanelSelectedEntity>() = HierarchyPanelSelectedEntity{};
    }
}

void EditorLayer::OpenScene(const std::string& path) {
    ClearScene();
    // No config: the project settings already loaded (window, input map...)
    // stay as they are instead of being replaced by this scene's copy.
    Serialization::SceneSerializer serializer;
    if (!serializer.Deserialize(path, *m_Registry, std::nullopt)) {
        log_error("[Editor] Could not read scene " + path + ". Opened it empty.");
    }
    m_Configuration->m_ActiveScenePath = path;
    MarkSceneSaved();
    log_info("[Editor] Opened scene " + ActiveSceneRelativePath());
}

void EditorLayer::CreateScene(const std::string& path) {
    ClearScene();
    m_Configuration->m_ActiveScenePath = path;
    SaveActiveScene();
    log_info("[Editor] Created scene " + ActiveSceneRelativePath());
}

void EditorLayer::SaveSceneAs(const std::string& path) {
    m_Configuration->m_ActiveScenePath = path;
    SaveActiveScene();
    log_info("[Editor] Saved scene as " + ActiveSceneRelativePath());
}

bool EditorLayer::IsStartupScene(const std::string& path) const {
    return m_Configuration && !m_Configuration->m_InitialScenePath.empty() &&
           SamePath(path, m_Configuration->m_InitialScenePath);
}

void EditorLayer::SetStartupScene(const std::string& path) const {
    const fs::path project = m_Configuration->m_ProjectFolder;
    std::error_code error;
    const fs::path relative = fs::relative(fs::weakly_canonical(path, error),
                                           fs::weakly_canonical(project, error), error);
    const std::string text = relative.generic_string();
    if (error || text.empty() || text.rfind("..", 0) == 0) {
        log_error("[Editor] The startup scene must be inside the project folder: " + path);
        return;
    }
    Serialization::ProjectFile file;
    Serialization::LoadProjectFile(project, file); // keep any other settings
    file.startupScene = text;
    if (!Serialization::SaveProjectFile(project, file)) {
        return;
    }
    m_Configuration->m_InitialScenePath = (project / relative).lexically_normal().string();
    log_info("[Editor] Startup scene set to " + text);
}

void EditorLayer::RunSceneAction(const SceneAction& action) {
    switch (action.kind) {
        case SceneAction::Kind::Open:
            OpenScene(action.path);
            break;
        case SceneAction::Kind::Create:
            CreateScene(action.path);
            break;
        case SceneAction::Kind::None:
            break;
    }
}

void EditorLayer::RequestSceneAction(SceneAction action) {
    if (!m_Registry || !m_Configuration || action.kind == SceneAction::Kind::None) {
        return;
    }
    if (action.kind == SceneAction::Kind::Open &&
        SamePath(action.path, m_Configuration->m_ActiveScenePath)) {
        return; // already open
    }
    if (HasUnsavedChanges()) {
        m_PendingSceneAction = std::move(action);
        m_OpenUnsavedPrompt = true;
        return;
    }
    RunSceneAction(action);
}

std::vector<std::string> EditorLayer::ListProjectScenes() const {
    std::vector<std::string> scenes;
    if (!m_Configuration || m_Configuration->m_ProjectFolder.empty()) {
        return scenes;
    }
    const fs::path root = m_Configuration->m_ProjectFolder;
    std::error_code error;
    fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, error);
    for (const fs::recursive_directory_iterator end; !error && it != end; it.increment(error)) {
        if (it->is_directory(error)) {
            if (IsSkippedFolder(it->path())) {
                it.disable_recursion_pending();
            }
            continue;
        }
        if (it->path().extension() == ".scene") {
            scenes.push_back(fs::relative(it->path(), root, error).generic_string());
        }
    }
    std::ranges::sort(scenes);
    return scenes;
}

void EditorLayer::OpenSceneNameDialog(const SceneNameDialog dialog, const std::string& folder) {
    m_NameDialog = dialog;
    m_NameDialogFolder = folder;
    m_NameDialogError.clear();
    if (dialog == SceneNameDialog::SaveAs && m_Configuration) {
        m_NameDialogText = fs::path(m_Configuration->m_ActiveScenePath).stem().string() + "_copy";
    } else {
        m_NameDialogText = "NewScene";
    }
    m_OpenNameDialog = true;
}

void EditorLayer::HandleSceneRequests() {
    if (!m_Registry || !m_Configuration) {
        return;
    }
    if (m_SavedSceneJsonStale && --s_SnapshotFramesLeft <= 0) {
        m_SavedSceneJson = CaptureSceneJson();
        m_SavedSceneJsonStale = false;
    }

    // Requests from panels (Assets panel double-click, context menus).
    if (!m_Configuration->m_RequestedScenePath.empty()) {
        RequestSceneAction({SceneAction::Kind::Open, m_Configuration->m_RequestedScenePath});
        m_Configuration->m_RequestedScenePath.clear();
    }
    if (!m_Configuration->m_RequestedStartupScene.empty()) {
        SetStartupScene(m_Configuration->m_RequestedStartupScene);
        m_Configuration->m_RequestedStartupScene.clear();
    }
    if (!m_Configuration->m_RequestedNewSceneFolder.empty()) {
        OpenSceneNameDialog(SceneNameDialog::NewScene, m_Configuration->m_RequestedNewSceneFolder);
        m_Configuration->m_RequestedNewSceneFolder.clear();
    }

    // Ctrl+S (Cmd+S on macOS: ImGui swaps the keys there).
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S) && SaveActiveScene()) {
        Gallery::UpdateProjectList(m_Configuration->m_ProjectFolder);
    }
}

void EditorLayer::DrawSceneMenuItems() {
    if (!m_Configuration) {
        return;
    }
    if (ImGui::MenuItem("New Scene...")) {
        OpenSceneNameDialog(SceneNameDialog::NewScene, m_Configuration->m_ProjectFolder);
    }
    const std::vector<std::string> scenes = ListProjectScenes();
    if (ImGui::BeginMenu("Open Scene", !scenes.empty())) {
        const std::string current = ActiveSceneRelativePath();
        for (const std::string& scene : scenes) {
            const fs::path full =
                (fs::path(m_Configuration->m_ProjectFolder) / scene).lexically_normal();
            const std::string label =
                IsStartupScene(full.string()) ? scene + "  " ICON_FA_FLAG " startup" : scene;
            if (ImGui::MenuItem(label.c_str(), nullptr, scene == current)) {
                RequestSceneAction({SceneAction::Kind::Open, full.string()});
            }
        }
        ImGui::EndMenu();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Save Scene", "Ctrl+S") && SaveActiveScene()) {
        Gallery::UpdateProjectList(m_Configuration->m_ProjectFolder);
    }
    if (ImGui::MenuItem("Save Scene As...")) {
        const fs::path active(m_Configuration->m_ActiveScenePath);
        OpenSceneNameDialog(SceneNameDialog::SaveAs, active.parent_path().string());
    }
    ImGui::Separator();
    const bool activeIsStartup = IsStartupScene(m_Configuration->m_ActiveScenePath);
    if (ImGui::MenuItem("Set as Startup Scene", nullptr, activeIsStartup, !activeIsStartup)) {
        // Save first so the game finds the file with its current content.
        SaveActiveScene();
        SetStartupScene(m_Configuration->m_ActiveScenePath);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip(activeIsStartup ? "The game starts with this scene"
                                          : "Make the game start with this scene");
    }
}

void EditorLayer::DrawSceneDialogs() {
    if (!m_Registry || !m_Configuration) {
        return;
    }

    // --- Unsaved changes ---
    if (m_OpenUnsavedPrompt) {
        ImGui::OpenPopup("Unsaved Changes");
        m_OpenUnsavedPrompt = false;
    }
    if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s has unsaved changes.", ActiveSceneRelativePath().c_str());
        ImGui::Text("Save them before leaving this scene?");
        ImGui::Spacing();
        if (ImGui::Button("Save")) {
            SaveActiveScene();
            RunSceneAction(m_PendingSceneAction);
            m_PendingSceneAction = {};
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Don't Save")) {
            RunSceneAction(m_PendingSceneAction);
            m_PendingSceneAction = {};
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            m_PendingSceneAction = {};
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // --- Scene name (New Scene / Save Scene As) ---
    const char* title = m_NameDialog == SceneNameDialog::SaveAs ? "Save Scene As" : "New Scene";
    if (m_OpenNameDialog) {
        ImGui::OpenPopup(title);
        m_OpenNameDialog = false;
    }
    if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        std::error_code error;
        const fs::path folderRelative =
            fs::relative(m_NameDialogFolder, m_Configuration->m_ProjectFolder, error);
        const std::string folderLabel = folderRelative.generic_string();
        ImGui::Text("Folder: %s",
                    error || folderLabel == "." ? "project root" : folderLabel.c_str());
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere();
        }
        const bool submitted =
            ImGui::InputText("Name", &m_NameDialogText, ImGuiInputTextFlags_EnterReturnsTrue);
        if (!m_NameDialogError.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", m_NameDialogError.c_str());
        }
        if (ImGui::Button("OK") || submitted) {
            std::string name = m_NameDialogText;
            if (!name.ends_with(".scene")) {
                name += ".scene";
            }
            const fs::path path = (fs::path(m_NameDialogFolder) / name).lexically_normal();
            if (m_NameDialogText.empty() ||
                m_NameDialogText.find_first_of("/\\") != std::string::npos) {
                m_NameDialogError = "Enter a name without slashes.";
            } else if (fs::exists(path, error)) {
                m_NameDialogError = name + " already exists.";
            } else {
                if (m_NameDialog == SceneNameDialog::SaveAs) {
                    SaveSceneAs(path.string());
                } else {
                    RequestSceneAction({SceneAction::Kind::Create, path.string()});
                }
                m_NameDialog = SceneNameDialog::None;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            m_NameDialog = SceneNameDialog::None;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void EditorLayer::BeginDockSpace() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(
        ImGuiStyleVar_FramePadding,
        ImVec2(ImGui::GetStyle().FramePadding.x, ImGui::GetStyle().FramePadding.y + 4.0f));

    ImGui::Begin("MainDockSpaceWindow", nullptr, window_flags);
    ImGui::PopStyleVar(4);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

    if (m_FirstTimeLayout) {
        m_FirstTimeLayout = false;
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

        ImGuiID dock_main_id = dockspace_id;
        ImGuiID dock_left_id =
            ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
        ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f,
                                                            nullptr, &dock_main_id);
        ImGuiID dock_down_id =
            ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.40f, nullptr, &dock_main_id);

        ImGui::DockBuilderDockWindow("State", dock_left_id);
        ImGui::DockBuilderDockWindow("Hierarchy", dock_left_id);
        ImGui::DockBuilderDockWindow("Inspector", dock_right_id);
        ImGui::DockBuilderDockWindow("Connections", dock_right_id);
        ImGui::DockBuilderDockWindow("Chat", dock_right_id);
        ImGui::DockBuilderDockWindow("Console", dock_down_id);
        ImGui::DockBuilderDockWindow("Assets", dock_down_id);
        ImGui::DockBuilderDockWindow("UI Palette", dock_down_id);
        ImGui::DockBuilderDockWindow("Scene", dock_main_id);
        ImGui::DockBuilderDockWindow(ICON_FA_GAMEPAD " Input Map", dock_main_id);
        ImGui::DockBuilderDockWindow(ICON_FA_GHOST " Waves", dock_main_id);
        ImGui::DockBuilderDockWindow("Preview", dock_main_id);
        ImGui::DockBuilderDockWindow("Script Editor", dock_main_id);

        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            DrawSceneMenuItems();
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Input Map")) {
            InputMappingPanel::s_IsOpen = true;
        }

        if (ImGui::MenuItem("Waves")) {
            WavePanel::s_IsOpen = true;
        }

        if (ImGui::MenuItem("UI Palette")) {
            UiPalettePanel::s_IsOpen = true;
        }

        if (ImGui::MenuItem("Settings")) {
            SettingsPanel::s_IsOpen = true;
        }

        if (ImGui::MenuItem("Script Editor")) {
            if (m_ScriptEditorPanel) {
                m_ScriptEditorPanel->Focus();
            }
        }

        float button_size = 30.0f;
        float spacing = 5.0f;
        float total_width = button_size * 2 + spacing;
        float center_pos = ImGui::GetWindowWidth() / 2.0f - total_width / 2.0f;

        if (ImGui::GetCursorPosX() < center_pos) {
            ImGui::SetCursorPosX(center_pos);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

        ImVec4 normalBg = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
        ImVec4 hoverBg = ImVec4(0.2f, 0.5f, 0.9f, 1.0f);
        ImVec4 redBg = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);

        bool isBuilding = SettingsPanel::s_IsBuilding.load();

        ImGui::PushStyleColor(ImGuiCol_Button, isBuilding ? redBg : normalBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, isBuilding ? redBg : hoverBg);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

        if (ImGui::Button(ICON_FA_HAMMER, ImVec2(button_size, 0))) {
            if (!isBuilding) {
                if (m_Configuration && m_Registry) {
                    SettingsPanel::StartBuild(*m_Configuration, *m_Registry);
                }
            }
        }
        ImGui::PopStyleColor(3);

        if (isBuilding) {
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Building %.0f%%", SettingsPanel::s_BuildProgress.load() * 100.f);
            }
        }

        ImGui::SameLine(0, spacing);

        ImGui::PushStyleColor(ImGuiCol_Button, normalBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverBg);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

        if (ImGui::Button(ICON_FA_PLAY, ImVec2(button_size, 0))) {
            const bool scriptsReady = m_ScriptEditorPanel == nullptr ||
                                      !m_ScriptEditorPanel->HasDirtyDocuments() ||
                                      m_ScriptEditorPanel->SaveAll();
            if (scriptsReady && m_Configuration) {
                // Save first: the game reads the scene file, not the editor's memory.
                SaveActiveScene();
#ifdef _WIN32
                const std::string exeFile = m_Configuration->m_BuildExecutableName + ".exe";
#else
                const std::string exeFile = m_Configuration->m_BuildExecutableName;
#endif
                std::string exePath = m_Configuration->m_ProjectFolder + "/" + exeFile;
                if (!std::filesystem::exists(exePath)) {
                    exePath = m_Configuration->m_ProjectFolder + "/build/" + exeFile;
                }
                if (!std::filesystem::exists(exePath)) {
                    exePath = m_Configuration->m_ProjectFolder + "/build/Games/R-Type/" + exeFile;
                }
                // Multi-config generators (e.g. Visual Studio) place the binary
                // in a <Config> subfolder.
                if (!std::filesystem::exists(exePath)) {
                    for (const char* cfg : {"Release", "Debug", "RelWithDebInfo"}) {
                        const std::string cand =
                            m_Configuration->m_ProjectFolder + "/build/" + cfg + "/" + exeFile;
                        if (std::filesystem::exists(cand)) {
                            exePath = cand;
                            break;
                        }
                        const std::string cand2 = m_Configuration->m_ProjectFolder +
                                                  "/build/Games/R-Type/" + cfg + "/" + exeFile;
                        if (std::filesystem::exists(cand2)) {
                            exePath = cand2;
                            break;
                        }
                    }
                }
                if (std::filesystem::exists(exePath)) {
                    // The game starts on the scene being edited (see
                    // Serialization::PickStartupScene in the generated main.cpp).
                    const std::string sceneArg = ActiveSceneRelativePath();
#ifdef _WIN32
                    // start detaches: system() returns immediately instead of
                    // freezing the editor while the game runs.
                    std::string cmd = "start \"\" /D \"" + m_Configuration->m_ProjectFolder +
                                      "\" \"" + exePath + "\" \"" + sceneArg + "\"";
#else
                    std::string cmd = "cd \"" + m_Configuration->m_ProjectFolder + "\" && \"" +
                                      exePath + "\" \"" + sceneArg + "\" &";
#endif
                    std::system(cmd.c_str());
                }
            }
        }
        ImGui::PopStyleColor(3); // Pop Play button style colors

        ImGui::SameLine(0, spacing);

        // Play Multiplayer: launch server + N clients for the current scene.
        const bool gamesRunning = !m_GameProcesses.empty();
        if (gamesRunning) {
            ImGui::PushStyleColor(ImGuiCol_Button, redBg);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, redBg);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            if (ImGui::Button(ICON_FA_STOP, ImVec2(button_size, 0))) {
                StopGames();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Stop all game processes");
            }
            ImGui::PopStyleColor(3);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, normalBg);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverBg);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            if (ImGui::Button(ICON_FA_USER_GROUP, ImVec2(button_size, 0))) {
                if (m_Configuration && m_Registry && SaveActiveScene()) {
                    // Scene path relative to the project folder (the game's CWD).
                    const std::string sceneName = ActiveSceneRelativePath();
                    std::vector<std::string> serverArgs{"--server", sceneName};
                    if (m_Configuration->m_MultiplayerServerHeadless) {
                        serverArgs.push_back("--headless");
                    }
                    LaunchGame(serverArgs);
                    const int maxClients =
                        static_cast<int>(Engine::Multiplayer::PlayerSlotState::kMaxPlayers);
                    const int clients =
                        std::clamp(m_Configuration->m_MultiplayerClientCount, 1, maxClients);
                    for (int i = 0; i < clients; ++i) {
                        // --client makes MultiplayerPlugin create a client host
                        // and connect to the server launched above. Without it
                        // the game would run in single-player mode.
                        LaunchGame({"--client", sceneName});
                    }
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Play Multiplayer (server + %d clients)",
                                  m_Configuration ? m_Configuration->m_MultiplayerClientCount : 0);
            }
            ImGui::PopStyleColor(3);
        }

        ImGui::PopStyleVar(2);

        // Open scene, right-aligned so it doesn't move the centered buttons.
        const std::string sceneLabel = ActiveSceneRelativePath();
        if (!sceneLabel.empty()) {
            const bool isStartup =
                m_Configuration && IsStartupScene(m_Configuration->m_ActiveScenePath);
            const std::string text = std::string(ICON_FA_FILE " ") + sceneLabel +
                                     (isStartup ? "  " ICON_FA_FLAG " startup" : "");
            const float textWidth = ImGui::CalcTextSize(text.c_str()).x;
            const float right =
                ImGui::GetWindowWidth() - textWidth - ImGui::GetStyle().ItemSpacing.x * 2.0f;
            if (ImGui::GetCursorPosX() < right) {
                ImGui::SetCursorPosX(right);
            }
            ImGui::TextDisabled("%s", text.c_str());
            if (ImGui::IsItemHovered() && m_Configuration) {
                ImGui::SetTooltip("Open scene: %s", m_Configuration->m_ActiveScenePath.c_str());
            }
        }

        ImGui::EndMenuBar();
    }

    // Same window as the menu bar, but outside the menu: popups opened from a
    // menu item would otherwise live in the menu's ID stack and never show.
    DrawSceneDialogs();
}

void EditorLayer::EndDockSpace() {
    ImGui::End(); // End MainDockSpaceWindow
}

bool EditorLayer::LaunchGame(const std::vector<std::string>& args) {
    if (!m_Configuration) {
        return false;
    }
#ifdef _WIN32
    const std::string exeFile = m_Configuration->m_BuildExecutableName + ".exe";
#else
    const std::string exeFile = m_Configuration->m_BuildExecutableName;
#endif
    std::string exePath = m_Configuration->m_ProjectFolder + "/" + exeFile;
    if (!std::filesystem::exists(exePath)) {
        exePath = m_Configuration->m_ProjectFolder + "/build/" + exeFile;
    }
    if (!std::filesystem::exists(exePath)) {
        for (const char* cfg : {"Release", "Debug", "RelWithDebInfo"}) {
            const std::string cand =
                m_Configuration->m_ProjectFolder + "/build/" + cfg + "/" + exeFile;
            if (std::filesystem::exists(cand)) {
                exePath = cand;
                break;
            }
        }
    }
    if (!std::filesystem::exists(exePath)) {
        return false;
    }

#ifdef _WIN32
    std::string cmdLine = "\"" + exePath + "\"";
    for (const auto& a : args) {
        cmdLine += " \"" + a + "\"";
    }
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    if (!CreateProcessA(nullptr, const_cast<char*>(cmdLine.c_str()), nullptr, nullptr, FALSE,
                        CREATE_NEW_CONSOLE, nullptr, m_Configuration->m_ProjectFolder.c_str(), &si,
                        &pi)) {
        return false;
    }
    CloseHandle(pi.hThread);
    m_GameProcesses.push_back(pi.hProcess);
#else
    // Build argv: exePath, args..., nullptr
    std::vector<std::string> argv;
    argv.push_back(exePath);
    for (const auto& a : args) {
        argv.push_back(a);
    }
    std::vector<char*> cargv;
    for (auto& a : argv) {
        cargv.push_back(const_cast<char*>(a.c_str()));
    }
    cargv.push_back(nullptr);

    pid_t pid = fork();
    if (pid < 0) {
        return false;
    }
    if (pid == 0) {
        setpgid(0, 0);
        execvp(exePath.c_str(), cargv.data());
        _exit(127); // execvp failed
    }
    m_GameProcesses.push_back(pid);
#endif
    return true;
}

void EditorLayer::StopGames() {
#ifdef _WIN32
    for (HANDLE h : m_GameProcesses) {
        if (h) {
            // Ask politely, then force-kill if it ignores us.
            EnumWindows(
                [](HWND wnd, LPARAM param) -> BOOL {
                    DWORD pid = 0;
                    GetWindowThreadProcessId(wnd, &pid);
                    if (pid == static_cast<DWORD>(param) && IsWindowVisible(wnd)) {
                        PostMessageA(wnd, WM_CLOSE, 0, 0);
                    }
                    return TRUE;
                },
                static_cast<LPARAM>(GetProcessId(h)));
            TerminateProcess(h, 0);
            CloseHandle(h);
        }
    }
#else
    for (pid_t pid : m_GameProcesses) {
        if (pid <= 0) {
            continue;
        }
        // Kill the whole process group the child created.
        kill(-pid, SIGTERM);
    }
    // Reap so they don't linger as zombies.
    for (pid_t pid : m_GameProcesses) {
        if (pid > 0) {
            waitpid(pid, nullptr, WNOHANG);
        }
    }
#endif
    m_GameProcesses.clear();
}
} // namespace Editor

namespace Editor {
void EditorLayer::SetRegistry(Engine::Registry& registry) {
    m_Registry = &registry;
}

void EditorLayer::SetConfiguration(EditorConfig& config) {
    m_Configuration = std::make_unique<EditorConfig>(config);
    if (m_Registry) {
        if (!m_Registry->HasResource<ProjectContext>()) {
            m_Registry->AddResource<ProjectContext>(config.m_ProjectFolder);
        } else {
            m_Registry->GetResource<ProjectContext>().projectFolder = config.m_ProjectFolder;
        }
        // Also expose EditorConfig as a resource for plugin editors.
        if (!m_Registry->HasResource<EditorConfig>()) {
            m_Registry->AddResource<EditorConfig>(config);
        } else {
            m_Registry->GetResource<EditorConfig>() = config;
        }
    }
}

void EditorLayer::OnRenderGame(Engine::Registry& registry) const {
    for (auto& panel : m_Panels) {
        if (auto sceneView = dynamic_cast<SceneViewPanel*>(panel.get())) {
            if (m_Configuration) {
                sceneView->RenderGame(registry, *m_Configuration);
            }
            break;
        }
    }
}
} // namespace Editor