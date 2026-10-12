#include "Panels/SettingsPanel.hpp"
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

    // Serialize current state
    std::string projFolder = std::filesystem::absolute(config.m_ProjectFolder).string();
    std::string scenePathStr = projFolder + "/test.scene";
    Serialization::SceneSerializer serializer;
    serializer.Serialize(scenePathStr, registry, config);

    std::string exeNameStr = config.m_BuildExecutableName;
    std::string winTitleStr = config.m_BuildWindowTitle;
    std::string mainCppPath = projFolder + "/main.cpp";

    std::ofstream mainOut(mainCppPath);
    mainOut << "#include \"Core/Application.hpp\"\n"
            << "#include <optional>\n"
            << "#include \"SceneSerializer.hpp\"\n"
            << "#include \"Plugins/PositionPlugin.hpp\"\n"
            << "#include \"Plugins/TextPlugin.hpp\"\n"
            << "#include \"Plugins/SpritePlugin.hpp\"\n"
            << "#include \"Plugins/ShapePlugin.hpp\"\n"
            << "#include \"Plugins/RotationPlugin.hpp\"\n"
            << "#include \"Plugins/ColliderPlugin.hpp\"\n"
            << "#include \"Inputs/InputManager.hpp\"\n"
            << "#include \"Inputs/Controller.hpp\"\n"
            << "#include <raylib.h>\n\n"
            << "int main() {\n"
            << "    ChangeDirectory(GetApplicationDirectory());\n"
            << "    Engine::ApplicationConfig config;\n"
            << "    config.windowTitle = \"" << winTitleStr << "\";\n"
            << "    config.windowWidth = " << config.windowWidth << ";\n"
            << "    config.windowHeight = " << config.windowHeight << ";\n"
            << "    config.targetFPS = " << config.targetFPS << ";\n\n"
            << "    Engine::Application app(config);\n"
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
            << "    colliderPlugin.Init(app.GetRegistry());\n\n"
            << "    auto& inputManager = app.GetRegistry().AddResource<Engine::InputManager>();\n"
            << "    auto controller = std::make_unique<Engine::Controller>();\n";

    for (const auto& [action, keybinds] : config.m_InputMap) {
        for (const auto& kb : keybinds) {
            mainOut << "    controller->addKeybind(\"" << action << "\", {" << kb.key << ", "
                    << kb.composeKey << ", static_cast<Engine::DeviceType>("
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

    std::ofstream cmakeOut(rootPath + "/.editor_build.cmake");
    cmakeOut << "add_executable(" << exeNameStr << " \"" << projFolder << "/main.cpp\")\n"
             << "target_link_libraries(" << exeNameStr << " PRIVATE Engine Serialization)\n"
             << "add_custom_command(TARGET " << exeNameStr << " POST_BUILD\n"
             << "    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:" << exeNameStr << "> \""
             << projFolder << "/" << exeNameStr << "\"\n"
             << ")\n";
    cmakeOut.close();

    std::string buildDir = projFolder + "/.build";
    std::string cmd = "cmake -B \"" + buildDir + "\" -S \"" + rootPath +
                      "\" -DEDITOR_BUILD=ON && make -C \"" + buildDir + "\" \"" + exeNameStr +
                      "\" -j$(nproc) 2>&1; rm -f \"" + rootPath + "/.editor_build.cmake\"";

    if (s_BuildThread.joinable()) {
        s_BuildThread.join();
    }

    s_BuildThread = std::thread([cmd]() {
        s_BuildProgress = 0.01f; // Show at least 1% so the user knows it started
        struct PipeDeleter {
            void operator()(FILE* p) const {
                pclose(p);
            }
        };
        std::unique_ptr<FILE, PipeDeleter> pipe(popen(cmd.c_str(), "r"));
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
