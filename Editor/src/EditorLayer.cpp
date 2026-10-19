#include "EditorLayer.hpp"
#include "EditorStyle.hpp"
#include "Gallery.hpp"
#include "Panels/AssetsPanel.hpp"
#include "Panels/ConnectionsPanel.hpp"
#include "Panels/ConsolePanel.hpp"
#include "Panels/HierarchyPanel.hpp"
#include "Panels/InputMappingPanel.hpp"
#include "Panels/InspectorPanel.hpp"
#include "Panels/PreviewPanel.hpp"
#include "Panels/SceneViewPanel.hpp"
#include "Panels/SettingsPanel.hpp"
#include "Resources/PlayerSlotState.hpp"
#include "SceneSerializer.hpp"
#include <algorithm>
#include <cstdlib>
#include <extras/FA6FreeSolidFontData.h>
#include <extras/IconsFontAwesome6.h>
#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h>
#include <rlImGui.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "BundledAssets.hpp"
#include "Gallery.hpp"
#include "Panels/StatePanel.hpp"
#include "SceneSerializer.hpp"

namespace Editor {
ImFont* g_IconFont = nullptr;

EditorLayer::EditorLayer() {
    AddPanel(std::make_unique<HierarchyPanel>());
    AddPanel(std::make_unique<InspectorPanel>());
    AddPanel(std::make_unique<ConnectionsPanel>());
    AddPanel(std::make_unique<ConsolePanel>());
    AddPanel(std::make_unique<AssetsPanel>());
    AddPanel(std::make_unique<SceneViewPanel>());
    AddPanel(std::make_unique<PreviewPanel>());
    AddPanel(std::make_unique<SettingsPanel>());
    AddPanel(std::make_unique<InputMappingPanel>());
    AddPanel(std::make_unique<StatePanel>());
}

EditorLayer::~EditorLayer() {
    Shutdown();
}

void EditorLayer::Init() {
    m_Registry->AddResource<HierarchyPanelSelectedEntity>();
    m_Registry->AddResource<StatePanelSelectedState>();
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

        static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
        ImFontConfig icons_config;
        icons_config.MergeMode = true;
        icons_config.PixelSnapH = true;
        icons_config.FontDataOwnedByAtlas = false;

        io.Fonts->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data,
                                                 fa_solid_900_compressed_size, 16.0f, &icons_config,
                                                 icons_ranges);

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
    serializer.Deserialize(m_Configuration->m_InitialScenePath, *m_Registry, *m_Configuration);

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

    BeginDockSpace();

    // Route selected entity from Hierarchy to Inspector
    HierarchyPanel* hierarchy = nullptr;
    InspectorPanel* inspector = nullptr;

    for (auto& panel : m_Panels) {
        if (auto h = dynamic_cast<HierarchyPanel*>(panel.get())) {
            hierarchy = h;
        }
        if (auto i = dynamic_cast<InspectorPanel*>(panel.get())) {
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
        ImGui::DockBuilderDockWindow("Console", dock_down_id);
        ImGui::DockBuilderDockWindow("Assets", dock_down_id);
        ImGui::DockBuilderDockWindow("Scene", dock_main_id);
        ImGui::DockBuilderDockWindow(ICON_FA_GAMEPAD " Input Map", dock_main_id);
        ImGui::DockBuilderDockWindow("Preview", dock_main_id);

        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                if (m_Registry && m_Configuration) {
                    Serialization::SceneSerializer serializer;
                    fs::path scenePath =
                        ResolveProjectPath(m_Configuration->m_ProjectFolder, "test.scene");
                    fs::create_directories(scenePath.parent_path());
                    serializer.Serialize(scenePath.string(), *m_Registry, *m_Configuration);
                    Gallery::UpdateProjectList(m_Configuration->m_ProjectFolder);
                }
            }
            // if (ImGui::MenuItem("Load Scene", "Ctrl+O"))
            // TODO: either pop up a list of projects/folders? or go back to the project gallery

            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Input Map")) {
            InputMappingPanel::s_IsOpen = true;
        }

        if (ImGui::MenuItem("Settings")) {
            SettingsPanel::s_IsOpen = true;
        }

        float button_size = 30.0f;
        float spacing = 5.0f;
        float total_width = (button_size * 2) + spacing;
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
            if (m_Configuration && m_Registry) {
                // Save scene first
                if (!m_Configuration->m_InitialScenePath.empty()) {
                    Serialization::SceneSerializer serializer;
                    serializer.Serialize(m_Configuration->m_InitialScenePath, *m_Registry,
                                         *m_Configuration);
                }
            }
            if (m_Configuration) {
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
#ifdef _WIN32
                    // start detaches: system() returns immediately instead of
                    // freezing the editor while the game runs.
                    std::string cmd = "start \"\" /D \"" + m_Configuration->m_ProjectFolder +
                                      "\" \"" + exePath + "\"";
#else
                    std::string cmd =
                        "cd \"" + m_Configuration->m_ProjectFolder + "\" && \"" + exePath + "\" &";
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
                if (m_Configuration && m_Registry && !m_Configuration->m_InitialScenePath.empty()) {
                    // Scene path relative to the project folder (the game's CWD).
                    const std::string sceneName =
                        std::filesystem::path(m_Configuration->m_InitialScenePath)
                            .filename()
                            .string();
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
        ImGui::EndMenuBar();
    }
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
                    if (pid == reinterpret_cast<DWORD>(param) && IsWindowVisible(wnd)) {
                        PostMessage(wnd, WM_CLOSE, 0, 0);
                    }
                    return TRUE;
                },
                reinterpret_cast<LPARAM>(GetProcessId(h)));
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

void EditorLayer::OnRenderGame(Engine::Registry& registry) {
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
