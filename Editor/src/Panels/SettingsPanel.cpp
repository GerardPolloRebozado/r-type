#include "Panels/SettingsPanel.hpp"
#include "Core/VirtualResolution.hpp"
#include "EditorConfig.hpp"
#include "SceneSerializer.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <iostream>
#include <memory>
#include <regex>

namespace Editor {
std::atomic<float> SettingsPanel::s_BuildProgress = 0.0f;
std::atomic<bool> SettingsPanel::s_IsBuilding = false;
bool SettingsPanel::s_IsOpen = false;
std::thread SettingsPanel::s_BuildThread;

SettingsPanel::~SettingsPanel() {
    if (s_BuildThread.joinable()) {
        s_BuildThread.detach(); // Allow it to finish in background if editor closes
    }
}

void SettingsPanel::StartBuild(EditorConfig& config, Engine::Registry& registry) {
    s_IsBuilding = true;
    s_BuildProgress = 0.0f;

    std::string projFolder = std::filesystem::absolute(config.m_ProjectFolder).generic_string();
    std::string scenePathStr = projFolder + "/test.scene";
    Serialization::SceneSerializer serializer;
    serializer.Serialize(scenePathStr, registry, config);

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
    std::string mainCppPath = projFolder + "/main.cpp";

    std::ofstream mainOut(mainCppPath);
    mainOut << "#include \"Core/Application.hpp\"\n"
            << "#include <optional>\n"
            << "#include \"SceneSerializer.hpp\"\n"
            << "#include \"Plugins/AudioPlugin.hpp\"\n"
            << "#include \"Plugins/HierarchyPlugin.hpp\"\n"
            << "#include \"Plugins/InputPlugin.hpp\"\n"
            << "#include \"Plugins/OpacityPlugin.hpp\"\n"
            << "#include \"Plugins/SignalPlugin.hpp\"\n"
            << "#include \"Plugins/PositionPlugin.hpp\"\n"
            << "#include \"Plugins/TextPlugin.hpp\"\n"
            << "#include \"Plugins/SpritePlugin.hpp\"\n"
            << "#include \"Plugins/ShapePlugin.hpp\"\n"
            << "#include \"Plugins/RotationPlugin.hpp\"\n"
            << "#include \"Plugins/ColliderPlugin.hpp\"\n"
            << "#include \"Plugins/CameraPlugin.hpp\"\n"
            << "#include \"Scripting/ScriptPlugin.hpp\"\n"
            << "#include \"Inputs/InputManager.hpp\"\n"
            << "#include \"Inputs/Controller.hpp\"\n"
            << "#include <raylib.h>\n\n"
            << "int main() {\n"
            << "    ChangeDirectory(GetApplicationDirectory());\n"
            << "    Engine::ApplicationConfig config;\n"
            << "    config.windowTitle = \"" << cppEscape(winTitleStr) << "\";\n"
            << "    config.windowWidth = " << config.windowWidth << ";\n"
            << "    config.windowHeight = " << config.windowHeight << ";\n"
            << "    config.virtualWidth = " << config.virtualWidth << ";\n"
            << "    config.virtualHeight = " << config.virtualHeight << ";\n"
            << "    config.targetFPS = " << config.targetFPS << ";\n\n"
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
            << "    Engine::ScriptPlugin scriptPlugin;\n"
            << "    scriptPlugin.Init(app.GetRegistry());\n\n"
            << "    auto& inputManager = app.GetRegistry().AddResource<Engine::InputManager>();\n"
            << "    auto controller = std::make_unique<Engine::Controller>();\n";

    for (const auto& [action, keybinds] : config.m_InputMap) {
        for (const auto& kb : keybinds) {
            mainOut << "    controller->addKeybind(\"" << cppEscape(action) << "\", {" << kb.key
                    << ", " << kb.composeKey << ", static_cast<Engine::DeviceType>("
                    << static_cast<int>(kb.type) << ")});\n";
        }
    }
    mainOut << "    inputManager.AddPlayer(1, std::move(controller));\n\n";

    mainOut << "    Serialization::SceneSerializer serializer;\n";

    if (std::filesystem::exists(projFolder) && std::filesystem::is_directory(projFolder)) {
        for (const auto& entry : std::filesystem::directory_iterator(projFolder)) {
            if (entry.path().extension() == ".scene") {
                mainOut << "    serializer.Deserialize(\"" << entry.path().filename().string()
                        << "\", app.GetRegistry(), std::nullopt);\n";
            }
        }
    }

    mainOut << "\n    app.Run();\n"
            << "    return 0;\n"
            << "}\n";
    mainOut.close();

    std::string rootPath = ".";
    if (!std::filesystem::exists("CMakeLists.txt") &&
        std::filesystem::exists("../CMakeLists.txt")) {
        rootPath = "..";
    }
    // Absolute forward-slash path: the build shell may run with any CWD, and
    // cmd.exe misparses forward slashes in `del`, so cleanup uses
    // `cmake -E rm -f` with this same absolute path on all platforms.
    rootPath = std::filesystem::absolute(rootPath).generic_string();

    std::ofstream cmakeOut(rootPath + "/.editor_build.cmake");
#ifdef _WIN32
    const std::string exeFileStr = exeNameStr + ".exe";
#else
    const std::string exeFileStr = exeNameStr;
#endif
    cmakeOut << "add_executable(" << exeNameStr << " \"" << projFolder << "/main.cpp\")\n"
             << "target_link_libraries(" << exeNameStr << " PRIVATE Engine Serialization)\n"
             << "add_custom_command(TARGET " << exeNameStr << " POST_BUILD\n"
             << "    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:" << exeNameStr << "> \""
             << projFolder << "/" << exeFileStr << "\"\n"
             << ")\n";
    cmakeOut.close();

    std::string buildDir = projFolder + "/.build";
    // Drop a stale build tree (e.g. configured with another generator before).
    // Guarded: only ever removes "<project>/.build".
    if (buildDir.size() > 7 && buildDir.compare(buildDir.size() - 7, 7, "/.build") == 0) {
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
    // Portable parallel build: `cmake --build --parallel` works with every
    // generator (make, ninja, MSVC...), unlike `make -C ... -j`.
    unsigned int jobs = std::thread::hardware_concurrency();
    if (jobs == 0) {
        jobs = 1;
    }
    const std::string parallelArgs = " --parallel " + std::to_string(jobs);
#ifdef _WIN32
    std::string cmd = "cmake -B \"" + buildDir + "\" -S \"" + rootPath + "\"" + toolchainArgs +
                      " -DEDITOR_BUILD=ON && cmake --build \"" + buildDir + "\" --target \"" +
                      exeNameStr + "\" --config Release" + parallelArgs +
                      " 2>&1 & cmake -E rm -f \"" + rootPath + "/.editor_build.cmake\"";
#else
    std::string cmd = "cmake -B \"" + buildDir + "\" -S \"" + rootPath + "\"" + toolchainArgs +
                      " -DEDITOR_BUILD=ON && cmake --build \"" + buildDir + "\" --target \"" +
                      exeNameStr + "\"" + parallelArgs + " 2>&1; cmake -E rm -f \"" + rootPath +
                      "/.editor_build.cmake\"";
#endif

    if (s_BuildThread.joinable()) {
        s_BuildThread.join();
    }

    s_BuildThread = std::thread([cmd, projFolder, buildDir, exeFileStr]() {
        s_BuildProgress = 0.01f; // Show at least 1% so the user knows it started
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
            s_IsBuilding = false;
            return;
        }

        std::array<char, 256> buffer;
        std::regex progressRegex("\\[\\s*(\\d+)%\\]");

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            std::string line(buffer.data());
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
        (void)exeFileStr;

        s_IsBuilding = false;
        s_BuildProgress = 1.0f;
    });
}

void SettingsPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    if (!s_IsOpen) {
        return;
    }

    if (ImGui::Begin(m_Name.c_str(), &s_IsOpen)) {
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
