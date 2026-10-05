#include "Panels/SettingsPanel.hpp"
#include "EditorConfig.hpp"
#include "Serialization/SceneSerializer.hpp"
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
    std::string projFolder = config.m_ProjectFolder;
    std::string scenePathStr = projFolder + "/test.scene";
    SceneSerializer serializer;
    serializer.Serialize(scenePathStr, registry, &config);

    std::string exeNameStr = config.m_BuildExecutableName;
    std::string winTitleStr = config.m_BuildWindowTitle;
    std::string mainCppPath = projFolder + "/main.cpp";

    std::ofstream mainOut(mainCppPath);
    mainOut << "#include \"Core/Application.hpp\"\n"
            << "#include \"Serialization/SceneSerializer.hpp\"\n"
            << "#include \"Plugins/PositionPlugin.hpp\"\n"
            << "#include \"Plugins/TextPlugin.hpp\"\n\n"
            << "int main() {\n"
            << "    Engine::ApplicationConfig config;\n"
            << "    config.windowTitle = \"" << winTitleStr << "\";\n"
            << "    config.windowWidth = " << config.windowWidth << ";\n"
            << "    config.windowHeight = " << config.windowHeight << ";\n"
            << "    config.targetFPS = " << config.targetFPS << ";\n\n"
            << "    Engine::Application app(config);\n"
            << "    Engine::PositionPlugin positionPlugin;\n"
            << "    positionPlugin.Init(app.GetRegistry());\n"
            << "    Engine::TextPlugin textPlugin;\n"
            << "    textPlugin.Init(app.GetRegistry());\n\n"
            << "    Editor::SceneSerializer serializer;\n"
            << "    serializer.Deserialize(\"test.scene\", app.GetRegistry(), nullptr);\n\n"
            << "    app.Run();\n"
            << "    return 0;\n"
            << "}\n";
    mainOut.close();

    std::string rootPath = ".";
    if (!std::filesystem::exists("CMakeLists.txt") &&
        std::filesystem::exists("../CMakeLists.txt")) {
        rootPath = "..";
    }

    std::ofstream cmakeOut(rootPath + "/.editor_build.cmake");
    cmakeOut << "file(GLOB_RECURSE EDITOR_PLUGINS Editor/src/Plugins/*.cpp)\n"
             << "add_executable(" << exeNameStr << " " << projFolder
             << "/main.cpp Editor/src/Serialization/SceneSerializer.cpp"
             << " Editor/src/Plugins/PositionPluginEditor.cpp "
                "Editor/src/Plugins/TextPluginEditor.cpp ${EDITOR_PLUGINS})\n"
             << "target_include_directories(" << exeNameStr << " PRIVATE Editor/include)\n"
             << "target_link_libraries(" << exeNameStr
             << " PRIVATE Engine rlImGui imgui nlohmann_json::nlohmann_json)\n"
             << "add_custom_command(TARGET " << exeNameStr << " POST_BUILD\n"
             << "    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:" << exeNameStr
             << "> ${CMAKE_CURRENT_SOURCE_DIR}/" << projFolder << "/" << exeNameStr << "\n"
             << ")\n";
    cmakeOut.close();

    std::string cmd = "cmake -B " + rootPath + "/build -S " + rootPath + " && make -C " + rootPath +
                      "/build " + exeNameStr + " -j$(nproc) 2>&1; rm -f " + mainCppPath;

    if (s_BuildThread.joinable()) {
        s_BuildThread.join();
    }

    s_BuildThread = std::thread([cmd]() {
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
