#include "Panels/SettingsPanel.hpp"
#include "Core/VirtualResolution.hpp"
#include "EditorConfig.hpp"
#include "Logging/Logger.hpp"
#include "ProjectFile.hpp"
#include "ProjectSystemsBuild.hpp"
#include "Resources/PlayerSlotState.hpp"
#include "SceneSerializer.hpp"
#include "ScriptEditorService.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <regex>

namespace Editor {
namespace {
// Startup scene as a path relative to the project folder, for the generated
// main.cpp (the game runs from the project folder). Uses project.json, then the
// editor's startup path, then the first .scene in the project folder, then
// "test.scene".
std::string StartupSceneRelativePath(const EditorConfig& config, const std::string& projFolder) {
    namespace fs = std::filesystem;
    std::error_code error;
    Serialization::ProjectFile project;
    if (Serialization::LoadProjectFile(projFolder, project) &&
        fs::is_regular_file(fs::path(projFolder) / project.startupScene, error)) {
        return project.startupScene;
    }
    if (!config.m_InitialScenePath.empty()) {
        fs::path scene(config.m_InitialScenePath);
        if (scene.is_relative()) {
            scene = fs::path(projFolder) / scene;
        }
        const fs::path relative = fs::relative(scene, projFolder, error);
        const std::string text = relative.generic_string();
        if (!error && !text.empty() && text.rfind("..", 0) != 0 && fs::exists(scene, error)) {
            return text;
        }
    }
    std::vector<std::string> found;
    for (const auto& entry : fs::directory_iterator(projFolder, error)) {
        if (entry.path().extension() == ".scene") {
            found.push_back(entry.path().filename().generic_string());
        }
    }
    if (!found.empty()) {
        std::sort(found.begin(), found.end());
        return found.front();
    }
    return "test.scene";
}
} // namespace

std::atomic<float> SettingsPanel::s_BuildProgress = 0.0f;
std::atomic<bool> SettingsPanel::s_IsBuilding = false;
std::mutex SettingsPanel::s_BuildMutex;
std::optional<SettingsPanel::BuildReport> SettingsPanel::s_BuildReport;
bool SettingsPanel::s_IsOpen = false;
std::thread SettingsPanel::s_BuildThread;

SettingsPanel::~SettingsPanel() {
    if (s_BuildThread.joinable()) {
        s_BuildThread.detach(); // Allow it to finish in background if editor closes
    }
}

void SettingsPanel::ReportBuildResult() {
    std::optional<BuildReport> report;
    {
        std::lock_guard<std::mutex> lock(s_BuildMutex);
        report.swap(s_BuildReport);
    }
    if (!report.has_value()) {
        return;
    }
    if (report->succeeded) {
        log_info("[Build] Game built: " + report->exePath);
        return;
    }
    log_error("[Build] Build failed. Last lines of the build output:");
    for (const std::string& line : report->output) {
        log_error("[Build]   " + line);
    }
}

void SettingsPanel::StartBuild(EditorConfig& config, Engine::Registry& registry) {
    s_IsBuilding = true;
    s_BuildProgress = 0.0f;

    std::string projFolder = std::filesystem::absolute(config.m_ProjectFolder).generic_string();
    // Save the open scene so the build uses what's on screen.
    if (!config.m_ActiveScenePath.empty()) {
        Serialization::SceneSerializer serializer;
        serializer.Serialize(config.m_ActiveScenePath, registry, config);
    }

    // WebAssembly builds run in the browser, which has no access to the
    // project folder on disk. Stage the runtime files (scenes, assets,
    // scripts, prefabs) into a clean folder that is embedded into the wasm
    // binary via Emscripten's --preload-file mechanism.
    std::string wasmStageDir;
    if (config.m_BuildWasm) {
        wasmStageDir = projFolder + "/.wasm_stage";
        std::error_code ec;
        std::filesystem::remove_all(wasmStageDir, ec);
        std::filesystem::create_directories(wasmStageDir, ec);
        auto stagePath = [&](const std::string& relative) {
            std::error_code e;
            std::filesystem::path src = std::filesystem::path(projFolder) / relative;
            if (!std::filesystem::exists(src, e)) {
                return;
            }
            std::filesystem::path dst = std::filesystem::path(wasmStageDir) / relative;
            std::filesystem::create_directories(dst.parent_path(), e);
            if (std::filesystem::is_directory(src, e)) {
                std::filesystem::copy(src, dst,
                                      std::filesystem::copy_options::recursive |
                                          std::filesystem::copy_options::overwrite_existing,
                                      e);
            } else {
                std::filesystem::copy_file(src, dst,
                                           std::filesystem::copy_options::overwrite_existing, e);
            }
        };
        stagePath("assets");
        stagePath("scripts");
        stagePath("Prefabs");
        stagePath("ui");
        stagePath("system");
        stagePath("project.json");
        stagePath("input.json");
        for (std::error_code itEc;
             auto& entry : std::filesystem::recursive_directory_iterator(
                 projFolder, std::filesystem::directory_options::skip_permission_denied, itEc)) {
            if (itEc) {
                break;
            }
            if (!entry.is_regular_file()) {
                continue;
            }
            if (entry.path().extension() != ".scene") {
                continue;
            }
            const std::filesystem::path relative =
                std::filesystem::relative(entry.path(), projFolder, itEc);
            if (itEc) {
                continue;
            }
            stagePath(relative.generic_string());
        }
    }

    // Escape a string for embedding in a C++ string literal.
    auto cppEscape = [](const std::string& in) {
        std::string out;
        out.reserve(in.size());
        for (const char c : in) {
            switch (c) {
                case '\\':
                    out += "\\\\";
                    break;
                case '"':
                    out += "\\\"";
                    break;
                case '\n':
                    out += "\\n";
                    break;
                default:
                    out += c;
                    break;
            }
        }
        return out;
    };

    std::string exeNameStr = config.m_BuildExecutableName;
    std::string winTitleStr = config.m_BuildWindowTitle;
    const bool buildWasm = config.m_BuildWasm;
    std::string mainCppPath = projFolder + "/main.cpp";

    std::ofstream mainOut(mainCppPath);
    mainOut << "#include \"Core/Application.hpp\"\n"
            << "#include \"SceneFiles.hpp\"\n"
            << "#include <string>\n"
            << "#include \"Plugins/AudioPlugin.hpp\"\n"
            << "#include \"Plugins/HierarchyPlugin.hpp\"\n"
            << "#include \"Plugins/InputPlugin.hpp\"\n"
            << "#include \"Plugins/MultiplayerPlugin.hpp\"\n"
            << "#include \"Plugins/ReplicationPlugin.hpp\"\n"
            << "#include \"Plugins/OpacityPlugin.hpp\"\n"
            << "#include \"Plugins/SignalPlugin.hpp\"\n"
            << "#include \"Plugins/PositionPlugin.hpp\"\n"
            << "#include \"Plugins/TextPlugin.hpp\"\n"
            << "#include \"Plugins/SpritePlugin.hpp\"\n"
            << "#include \"Plugins/ShapePlugin.hpp\"\n"
            << "#include \"Plugins/RotationPlugin.hpp\"\n"
            << "#include \"Plugins/ColliderPlugin.hpp\"\n"
            << "#include \"Plugins/CameraPlugin.hpp\"\n"
            << "#include \"Plugins/AnimationPlugin.hpp\"\n"
            << "#include \"Plugins/WavePlugin.hpp\"\n"
            << "#include \"Scripting/ScriptPlugin.hpp\"\n"
            << "#include \"UI/UiPlugin.hpp\"\n"
            << "#include \"Inputs/InputManager.hpp\"\n"
            << "#include \"Inputs/Controller.hpp\"\n"
            << "#include <vector>\n"
            << "#include <raylib.h>\n"
            << Editor::ProjectSystemsMainCppSnippet(projFolder) << "\n"
            << "int main(int argc, char *argv[]) {\n"
            << "    ChangeDirectory(GetApplicationDirectory());\n"
            << "    Engine::ApplicationConfig config;\n"
            << "    config.windowTitle = \"" << cppEscape(winTitleStr) << "\";\n"
            << "    config.windowWidth = " << config.windowWidth << ";\n"
            << "    config.windowHeight = " << config.windowHeight << ";\n"
            << "    config.virtualWidth = " << config.virtualWidth << ";\n"
            << "    config.virtualHeight = " << config.virtualHeight << ";\n"
            << "    config.targetFPS = " << config.targetFPS << ";\n\n"

            << "    config.args = std::vector<std::string>(argv + 1, argv + argc);\n"
            << "    for (const auto& a : config.args) {\n"
            << "        if (a == \"--headless\") config.headless = true;\n"
            << "    }\n"
            << "    Engine::Application app(config);\n"
            << "    // HierarchyPlugin before PositionPlugin: parent deltas must\n"
            << "    // propagate before prevX/prevY are snapshotted.\n"
            << "    Engine::HierarchyPlugin hierarchyPlugin;\n"
            << "    hierarchyPlugin.Init(app.GetRegistry());\n"
            << "    Engine::PositionPlugin positionPlugin;\n"
            << "    positionPlugin.Init(app.GetRegistry());\n"
            << "    Engine::TextPlugin textPlugin;\n"
            << "    textPlugin.Init(app.GetRegistry());\n"
            << "    Engine::SpritePlugin spritePlugin;\n"
            << "    spritePlugin.Init(app.GetRegistry());\n"
            << "    Engine::ShapePlugin shapePlugin;\n"
            << "    shapePlugin.Init(app.GetRegistry());\n"
            << "    Engine::RotationPlugin rotationPlugin;\n"
            << "    rotationPlugin.Init(app.GetRegistry());\n"
            << "    Engine::ColliderPlugin colliderPlugin;\n"
            << "    colliderPlugin.Init(app.GetRegistry());\n"
            << "    Engine::OpacityPlugin opacityPlugin;\n"
            << "    opacityPlugin.Init(app.GetRegistry());\n"
            << "    Engine::SignalPlugin signalPlugin;\n"
            << "    signalPlugin.Init(app.GetRegistry());\n"
            << "    Engine::InputPlugin inputPlugin;\n"
            << "    inputPlugin.Init(app.GetRegistry());\n"
            << "    Engine::AudioPlugin audioPlugin;\n"
            << "    audioPlugin.Init(app.GetRegistry());\n"
            << "    Engine::CameraPlugin cameraPlugin;\n"
            << "    cameraPlugin.Init(app.GetRegistry());\n"
            << "    Engine::AnimationPlugin animationPlugin;\n"
            << "    animationPlugin.Init(app.GetRegistry());\n"
            << "    Engine::WavePlugin wavePlugin;\n"
            << "    wavePlugin.Init(app.GetRegistry());\n"
            << "    Engine::ScriptPlugin scriptPlugin;\n"
            << "    scriptPlugin.Init(app.GetRegistry());\n"
            << "    Engine::UiPlugin().Init(app.GetRegistry());\n"
            << "    Engine::Multiplayer::ReplicationPlugin replicationPlugin;\n"
            << "    replicationPlugin.Init(app.GetRegistry());\n"
            << "    Engine::Multiplayer::MultiplayerPlugin multiplayerPlugin;\n"
            << "    multiplayerPlugin.Init(app.GetRegistry());\n\n"
            << "    auto& inputManager = app.GetRegistry().AddResource<Engine::InputManager>();\n"
            << "    auto controller = std::make_unique<Engine::Controller>();\n";

    for (const auto& [action, binding] : config.m_InputMap) {
        for (const auto& kb : binding.keybinds) {
            mainOut << "    controller->addKeybind(\"" << cppEscape(action) << "\", {" << kb.key
                    << ", " << kb.composeKey << ", static_cast<Engine::DeviceType>("
                    << static_cast<int>(kb.type) << ")}, " << (binding.networked ? "true" : "false")
                    << ");\n";
        }
    }
    mainOut << "    inputManager.AddPlayer(1, std::move(controller));\n\n";

    if (std::filesystem::exists(std::filesystem::path(projFolder) / kProjectRegisterHeader)) {
        mainOut << "    RegisterScaffoldedSystems(app.GetRegistry());\n\n";
    }

    // Only the startup scene is loaded. Other scenes are loaded later with
    // Scene.load(...) from Lua or SceneManager::Load from C++.
    mainOut
        << "    // Startup scene: a .scene argument wins (e.g. ./" << cppEscape(exeNameStr)
        << " Level2.scene),\n"
        << "    // otherwise the project's startup scene.\n"
        << "    const std::string startupScene = Serialization::PickStartupScene(config.args, \""
        << cppEscape(StartupSceneRelativePath(config, projFolder)) << "\");\n"
        << "    Serialization::EnableSceneFiles(app.GetRegistry()).Load(startupScene);\n";

    mainOut << "\n    app.Run();\n"
            << "    return 0;\n"
            << "}\n";
    mainOut.close();

    // Engine sources: the folder the editor was built from (baked in at
    // compile time), so the build works whatever folder the editor was
    // started from (Finder, an IDE, a terminal). Falls back to the old
    // working-directory guess if that folder moved.
    std::string rootPath = ".";
#ifdef RTYPE_SOURCE_DIR
    if (std::filesystem::exists(std::filesystem::path(RTYPE_SOURCE_DIR) / "CMakeLists.txt")) {
        rootPath = RTYPE_SOURCE_DIR;
    } else
#endif
        if (!std::filesystem::exists("CMakeLists.txt") &&
            std::filesystem::exists("../CMakeLists.txt")) {
        rootPath = "..";
    }
    // Absolute forward-slash path: the build shell may run with any CWD, and
    // cmd.exe misparses forward slashes in `del`, so cleanup uses
    // `cmake -E rm -f` with this same absolute path on all platforms.
    rootPath = std::filesystem::absolute(rootPath).generic_string();

    if (config.m_BuildWasm && !wasmStageDir.empty()) {
        std::error_code e;
        std::filesystem::path src = std::filesystem::path(rootPath) / "UI" / "lua" / "templates";
        if (std::filesystem::exists(src, e)) {
            std::filesystem::path dst = std::filesystem::path(wasmStageDir) / rootPath.substr(1) /
                                        "UI" / "lua" / "templates";
            std::filesystem::create_directories(dst.parent_path(), e);
            std::filesystem::copy(src, dst,
                                  std::filesystem::copy_options::recursive |
                                      std::filesystem::copy_options::overwrite_existing,
                                  e);
        }
    }

    const std::string projectSystemsCmake = Editor::ProjectSystemsEditorBuildSnippet(projFolder);
    const std::filesystem::path projectSystemsDir =
        Editor::ProjectSystemsDirectory(std::filesystem::path(projFolder));

    std::ofstream cmakeOut(projFolder + "/CMakeLists.txt");
    cmakeOut << "# DO NOT MODIFY, AUTOGENERATED CMAKE\n";
    cmakeOut << "# This file is generated by the editor. Changes will be overwritten.\n";
    cmakeOut << "\n";
    const std::string exeFileStr = buildWasm ? std::string("index.html")
#ifdef _WIN32
                                             : (exeNameStr + ".exe");
#else
                                             : exeNameStr;
#endif
    cmakeOut << projectSystemsCmake << "add_executable(" << exeNameStr << " \"main.cpp\" "
             << "${RTYPE_PROJECT_SYSTEM_SOURCES})\n";
    if (buildWasm) {
        cmakeOut << "set_target_properties(" << exeNameStr
                 << " PROPERTIES OUTPUT_NAME index SUFFIX \".html\")\n";
        cmakeOut << "target_compile_options(" << exeNameStr << " PRIVATE -fexceptions)\n";
        cmakeOut << "target_link_options(" << exeNameStr
                 << " PRIVATE -sASYNCIFY -fexceptions --preload-file \"" << wasmStageDir
                 << "@/\")\n";
    }
    if (std::filesystem::exists(projectSystemsDir)) {
        cmakeOut << "target_include_directories(" << exeNameStr << " PRIVATE \""
                 << projectSystemsDir.string() << "\")\n";
    }
    cmakeOut << "target_link_libraries(" << exeNameStr << " PRIVATE Engine Serialization UI)\n";
    cmakeOut << "\n";
    cmakeOut << "if(NOT CMAKE_SYSTEM_NAME STREQUAL \"Emscripten\")\n";
    cmakeOut << "    set_target_properties(" << exeNameStr << " PROPERTIES\n";
    cmakeOut << "        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin\n";
    cmakeOut << "    )\n";
    cmakeOut << "    add_custom_command(TARGET " << exeNameStr << " POST_BUILD\n";
    cmakeOut << "        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:" << exeNameStr
             << "> \"${CMAKE_CURRENT_SOURCE_DIR}/" << exeFileStr << "\"\n";
    cmakeOut << "    )\n";
    cmakeOut << "endif()\n";
    cmakeOut.close();

    std::string buildDir = projFolder + "/build";
    // Drop a stale build tree (e.g. configured with another generator before).
    if (buildDir.size() > 6 && buildDir.compare(buildDir.size() - 6, 6, "/build") == 0) {
        std::error_code ec;
        std::filesystem::remove_all(buildDir, ec);
    }
    // Reuse the editor's own toolchain so exported games build with the same
    // compiler that built the editor (baked at configure time). Otherwise
    // Windows silently falls back to Visual Studio.
    std::string toolchainArgs;
#ifdef EDITOR_CMAKE_GENERATOR
    toolchainArgs += " -G \"" + std::string(EDITOR_CMAKE_GENERATOR) + "\"";
#endif
#ifdef EDITOR_CMAKE_C_COMPILER
    if (!std::string(EDITOR_CMAKE_C_COMPILER).empty()) {
        toolchainArgs += " -DCMAKE_C_COMPILER=\"" + std::string(EDITOR_CMAKE_C_COMPILER) + "\"";
    }
#endif
#ifdef EDITOR_CMAKE_CXX_COMPILER
    if (!std::string(EDITOR_CMAKE_CXX_COMPILER).empty()) {
        toolchainArgs += " -DCMAKE_CXX_COMPILER=\"" + std::string(EDITOR_CMAKE_CXX_COMPILER) + "\"";
    }
#endif
#ifdef EDITOR_CMAKE_MAKE_PROGRAM
    if (!std::string(EDITOR_CMAKE_MAKE_PROGRAM).empty()) {
        toolchainArgs += " -DCMAKE_MAKE_PROGRAM=\"" + std::string(EDITOR_CMAKE_MAKE_PROGRAM) + "\"";
    }
#endif
#ifdef EDITOR_CMAKE_PREFIX_PATH
    // Same dependency locations as the editor's own build (e.g. a Lua
    // installed outside the default paths). Baked with '|' between entries.
    if (!std::string(EDITOR_CMAKE_PREFIX_PATH).empty()) {
        std::string prefixPath = EDITOR_CMAKE_PREFIX_PATH;
        std::replace(prefixPath.begin(), prefixPath.end(), '|', ';');
        toolchainArgs += " -DCMAKE_PREFIX_PATH=\"" + prefixPath + "\"";
    }
#endif
    // Portable parallel build: `cmake --build --parallel` works with every
    // generator (make, ninja, MSVC...), unlike `make -C ... -j`.
    unsigned int jobs = std::thread::hardware_concurrency();
    if (jobs == 0) {
        jobs = 1;
    }
    const std::string parallelArgs = " --parallel " + std::to_string(jobs);
    // Debug builds so dev can use gdb
    const std::string buildTypeArg = config.m_BuildDebugSymbols ? " -DCMAKE_BUILD_TYPE=Debug" : "";
#ifdef _WIN32
    const std::string buildConfigArg =
        config.m_BuildDebugSymbols ? " --config Debug" : " --config Release";
#else
    const std::string buildConfigArg;
#endif
    // Configure and build are grouped so both outputs (including configure
    // errors such as a missing library) reach the pipe and the Console.
    const std::string configureProgram =
        buildWasm ? std::string("emcmake cmake") : std::string("cmake");
    const std::string wasmToolchainArgs = buildWasm ? std::string() : toolchainArgs;
#ifdef _WIN32
    std::string cmd = "(" + configureProgram + " -B \"" + buildDir + "\" -S \"" + rootPath + "\"" +
                      wasmToolchainArgs + " -DEDITOR_BUILD=ON -DGAME_PROJECT_DIR=\"" + projFolder +
                      "\"" + buildTypeArg + " && cmake --build \"" + buildDir + "\" --target \"" +
                      exeNameStr + "\"" + buildConfigArg + parallelArgs + ") 2>&1";
#else
    std::string cmd = "(" + configureProgram + " -B \"" + buildDir + "\" -S \"" + rootPath + "\"" +
                      wasmToolchainArgs + " -DEDITOR_BUILD=ON -DGAME_PROJECT_DIR=\"" + projFolder +
                      "\"" + buildTypeArg + " && cmake --build \"" + buildDir + "\" --target \"" +
                      exeNameStr + "\"" + buildConfigArg + parallelArgs + ") 2>&1";
#endif

    if (s_BuildThread.joinable()) {
        s_BuildThread.join();
    }

    {
        std::lock_guard lock(s_BuildMutex);
        s_BuildReport.reset();
    }
    s_BuildThread = std::thread([cmd, projFolder, buildDir, exeFileStr, buildWasm, wasmStageDir]() {
        s_BuildProgress = 0.01f; // Show at least 1% so the user knows it started
        const auto startTime = std::filesystem::file_time_type::clock::now();
        BuildReport report;
        report.exePath = projFolder + "/" + exeFileStr;
        struct PipeDeleter {
            void operator()(FILE* p) const {
#ifdef _WIN32
                _pclose(p);
#else
                pclose(p);
#endif
            }
        };
#ifdef _WIN32
        std::unique_ptr<FILE, PipeDeleter> pipe(_popen(cmd.c_str(), "r"));
#else
        std::unique_ptr<FILE, PipeDeleter> pipe(popen(cmd.c_str(), "r"));
#endif
        if (!pipe) {
            report.output.push_back("Could not start the build command.");
            std::lock_guard<std::mutex> lock(s_BuildMutex);
            s_BuildReport = std::move(report);
            s_IsBuilding = false;
            return;
        }

        std::array<char, 256> buffer;
        std::regex progressRegex("\\[\\s*(\\d+)%\\]");

        std::string pending;
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            std::string line(buffer.data());
            // Keep the last lines for the Console (fgets may split long lines).
            pending += line;
            if (!pending.empty() && pending.back() == '\n') {
                pending.pop_back();
                report.output.push_back(std::move(pending));
                pending.clear();
                if (report.output.size() > kBuildOutputLines) {
                    report.output.erase(report.output.begin());
                }
            }
            std::smatch match;
            if (std::regex_search(line, match, progressRegex)) {
                if (match.size() > 1) {
                    float progress = std::stof(match[1].str()) / 100.0f;
                    s_BuildProgress = progress;
                }
            }
        }

        // The game exe runs from the project folder, so its runtime DLLs must
        // live next to it (Windows has no rpath). Best effort: lua54.dll plus
        // the MinGW runtime when the editor itself uses MinGW.
        auto copyDll = [&](const std::string& src) {
            if (src.empty()) {
                return;
            }
            std::error_code ec;
            if (!std::filesystem::exists(src, ec) || ec) {
                return;
            }
            std::filesystem::copy_file(
                src, projFolder + "/" + std::filesystem::path(src).filename().string(),
                std::filesystem::copy_options::overwrite_existing, ec);
        };
#ifdef EDITOR_LUA_DLL
        copyDll(EDITOR_LUA_DLL);
#endif
#ifdef EDITOR_MINGW_BIN_DIR
        const std::string mingwBin = EDITOR_MINGW_BIN_DIR;
        if (!mingwBin.empty()) {
            copyDll(mingwBin + "/libwinpthread-1.dll");
            copyDll(mingwBin + "/libstdc++-6.dll");
            copyDll(mingwBin + "/libgcc_s_seh-1.dll");
        }
#endif
        pipe.reset(); // wait for the command to finish

        // WebAssembly builds also produce a .js loader and a .wasm binary next
        // to the generated index.html; copy them to the project folder too so
        // the exported game can run from there.
        if (buildWasm) {
            std::string jsName = exeNameStr + ".js";
            std::string wasmName = exeNameStr + ".wasm";
            std::string dataName = exeNameStr + ".data";
            for (const char* name : {jsName.c_str(), wasmName.c_str(), dataName.c_str()}) {
                std::error_code ec;
                std::filesystem::copy_file(buildDir + "/" + name, projFolder + "/" + name,
                                           std::filesystem::copy_options::overwrite_existing, ec);
            }
            // Replace Emscripten's default shell (which shows a logo and a
            // fullscreen button) with a minimal page that only hosts the game.
            std::ofstream htmlOut(projFolder + "/index.html", std::ios::trunc);
            htmlOut << "<!DOCTYPE html>\n"
                    << "<html>\n"
                    << "<head>\n"
                    << "  <meta charset=\"utf-8\">\n"
                    << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, "
                       "maximum-scale=1.0, user-scalable=no\">\n"
                    << "  <title>Game</title>\n"
                    << "  <style>\n"
                    << "    html, body { margin: 0; padding: 0; width: 100%; height: 100%; "
                    << "overflow: hidden; background: #000; }\n"
                    << "    #canvas { display: block; width: 100vw; height: 100vh; }\n"
                    << "  </style>\n"
                    << "</head>\n"
                    << "<body>\n"
                    << "  <canvas id=\"canvas\" tabindex=\"-1\"></canvas>\n"
                    << "  <script>\n"
                    << "    window.Module = { canvas: document.getElementById('canvas') };\n"
                    << "  </script>\n"
                    << "  <script src=\"" << jsName << "\"></script>\n"
                    << "</body>\n"
                    << "</html>\n";
            htmlOut.close();
            std::error_code sec;
            std::filesystem::remove_all(wasmStageDir, sec);
        }

        // Success = the game was copied to the project folder by this build.
        // (The shell's exit code is not portable across sh and cmd.exe.)
        std::error_code ec;
        report.succeeded = std::filesystem::exists(report.exePath, ec) &&
                           std::filesystem::last_write_time(report.exePath, ec) >= startTime;
        {
            std::lock_guard<std::mutex> lock(s_BuildMutex);
            s_BuildReport = std::move(report);
        }

        s_IsBuilding = false;
        s_BuildProgress = 1.0f;
    });
}

void SettingsPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    if (!s_IsOpen) {
        return;
    }

    if (ImGui::Begin(m_Name.c_str(), &s_IsOpen)) {
        ImGui::Text("IDE Settings");
        ImGui::Separator();

        ImGui::Text("Script Editor");
        ImGui::Separator();

        static float scriptFontSize = 15.0f;
        if (ImGui::SliderFloat("Font Size", &scriptFontSize, 8.0f, 32.0f, "%.0f")) {
            config.m_ScriptFontSize = scriptFontSize;
            if (registry.HasResource<ScriptEditorService>()) {
                auto& service = registry.GetResource<ScriptEditorService>();
                service.RequestFontSizeChange(scriptFontSize);
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Changes apply to all open script editors");
        }

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::Text("Build Configuration");
        ImGui::Separator();

        static std::array<char, 256> exeName;
        static std::array<char, 256> winTitle;
        static bool initialized = false;

        if (!initialized) {
            exeName.fill('\0');
            winTitle.fill('\0');
            std::copy_n(config.m_BuildExecutableName.begin(),
                        std::min(config.m_BuildExecutableName.size(), exeName.size() - 1),
                        exeName.begin());
            std::copy_n(config.m_BuildWindowTitle.begin(),
                        std::min(config.m_BuildWindowTitle.size(), winTitle.size() - 1),
                        winTitle.begin());
            initialized = true;
        }

        if (ImGui::InputText("Executable Name", exeName.data(), exeName.size())) {
            config.m_BuildExecutableName = exeName.data();
        }
        if (ImGui::InputText("Window Title", winTitle.data(), winTitle.size())) {
            config.m_BuildWindowTitle = winTitle.data();
        }

        // Debug builds pass CMAKE_BUILD_TYPE=Debug so the exported game keeps
        // its DWARF symbols (-g, no optimization) and can be inspected with gdb.
        ImGui::Checkbox("Debug Build (gdb symbols)", &config.m_BuildDebugSymbols);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Builds the game with -g (Debug). Use it to debug the "
                              "exported executable with gdb.");
        }

        ImGui::Checkbox("Build to WebAssembly (WASM)", &config.m_BuildWasm);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Compiles the game to WebAssembly with Emscripten, "
                              "enabling asyncify and exceptions, and generates an "
                              "index.html that loads the wasm code.");
        }

        ImGui::Spacing();
        ImGui::Text("Multiplayer Playtest");
        ImGui::Separator();

        ImGui::InputInt("Client Count", &config.m_MultiplayerClientCount);
        const int maxClients = static_cast<int>(Engine::Multiplayer::PlayerSlotState::kMaxPlayers);
        if (config.m_MultiplayerClientCount < 1) {
            config.m_MultiplayerClientCount = 1;
        }
        if (config.m_MultiplayerClientCount > maxClients) {
            config.m_MultiplayerClientCount = maxClients;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Between 1 and %d (one color per player)", maxClients);
        }
        ImGui::Checkbox("Headless Server", &config.m_MultiplayerServerHeadless);

        ImGui::Spacing();
        ImGui::Text("Window Settings");
        ImGui::Separator();

        ImGui::InputInt("Window Width", &config.windowWidth);
        ImGui::InputInt("Window Height", &config.windowHeight);

        int virtualWidth =
            Engine::VirtualResolution::ResolveAxis(config.virtualWidth, config.windowWidth);
        int virtualHeight =
            Engine::VirtualResolution::ResolveAxis(config.virtualHeight, config.windowHeight);
        if (ImGui::InputInt("Virtual Width", &virtualWidth)) {
            config.virtualWidth = virtualWidth;
        }
        if (ImGui::InputInt("Virtual Height", &virtualHeight)) {
            config.virtualHeight = virtualHeight;
        }
        ImGui::TextDisabled("Game canvas. Scaled to fit the window.");

        ImGui::InputInt("Target FPS", &config.targetFPS);

        ImGui::Spacing();
        ImGui::Separator();

        if (s_BuildProgress.load() >= 1.0f && !s_IsBuilding.load()) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Previous Build Complete!");
        }
    }
    ImGui::End();
}
} // namespace Editor

namespace Editor {
const std::string& SettingsPanel::GetName() const {
    return m_Name;
}
} // namespace Editor