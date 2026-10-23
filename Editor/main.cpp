#include "BundledAssets.hpp"
#include "Core/Application.hpp"
#include "Core/EditorMode.hpp"
#include "EditorLayer.hpp"
#include "EditorStyle.hpp"
#include "Gallery.hpp"
#include "Logging/Logger.hpp"
#include "Plugins/AudioPlugin.hpp"
#include "Plugins/CameraPlugin.hpp"
#include "Plugins/ColliderPlugin.hpp"
#include "Plugins/HierarchyPlugin.hpp"
#include "Plugins/InputPlugin.hpp"
#include "Plugins/OpacityPlugin.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/ReplicationPlugin.hpp"
#include "Plugins/RotationPlugin.hpp"
#include "Plugins/ShapePlugin.hpp"
#include "Plugins/SignalPlugin.hpp"
#include "Plugins/SpritePlugin.hpp"
#include "Plugins/TextPlugin.hpp"
#include "Scaffold.hpp"
#include "Scripting/ScriptPlugin.hpp"
#include "imgui.h"
#include "raylib.hpp"
#include "rlImGui.h"

#include <extras/FA6FreeSolidFontData.h>
#include <extras/IconsFontAwesome6.h>
#include <filesystem>

using std::string;

void RunEditor(const string& scenePathString) {
    Editor::EditorConfig config;

    std::filesystem::path projectFolder = scenePathString;
    projectFolder = projectFolder.parent_path();

    config.windowTitle = "Engine Editor";
    config.m_ProjectFolder = projectFolder.string();
    config.m_InitialScenePath = scenePathString;
    config.m_ActiveScenePath =
        std::filesystem::absolute(scenePathString).lexically_normal().string();
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.maximized = true;

    Editor::EditorLayer editorLayer;

    config.onUI = [&editorLayer]() { editorLayer.OnUpdate(); };
    config.customRender = [&editorLayer](Engine::Registry& registry) {
        editorLayer.OnRenderGame(registry);
    };

    Engine::Application app(config);

    // Initialize engine plugins (registers Position component & system)
    // NOTE: HierarchyPlugin must init before PositionPlugin so
    // HierarchySystem propagates parent deltas before PositionSystem
    // snapshots prevX/prevY (see HierarchyPlugin docs). Otherwise children
    // never follow their parents.
    Engine::HierarchyPlugin hierarchyPlugin;
    hierarchyPlugin.Init(app.GetRegistry());
    Engine::PositionPlugin positionPlugin;
    positionPlugin.Init(app.GetRegistry());
    Engine::TextPlugin text_plugin;
    text_plugin.Init(app.GetRegistry());
    Engine::SpritePlugin spritePlugin;
    spritePlugin.Init(app.GetRegistry());
    Engine::ShapePlugin shapePlugin;
    shapePlugin.Init(app.GetRegistry());
    Engine::RotationPlugin rotationPlugin;
    rotationPlugin.Init(app.GetRegistry());
    Engine::Multiplayer::ReplicationPlugin replicationPlugin;
    replicationPlugin.Init(app.GetRegistry());
    Engine::ColliderPlugin colliderPlugin;
    colliderPlugin.Init(app.GetRegistry());
    Engine::OpacityPlugin opacityPlugin;
    opacityPlugin.Init(app.GetRegistry());
    Engine::SignalPlugin signalPlugin;
    signalPlugin.Init(app.GetRegistry());
    Engine::InputPlugin inputPlugin;
    inputPlugin.Init(app.GetRegistry());
    Engine::AudioPlugin audioPlugin;
    audioPlugin.Init(app.GetRegistry());
    Engine::ScriptPlugin scriptPlugin;
    scriptPlugin.Init(app.GetRegistry());
    Engine::CameraPlugin cameraPlugin;
    cameraPlugin.Init(app.GetRegistry());

    // This registry is edited, never played
    app.GetRegistry().AddResource<Engine::EditorMode>();

    editorLayer.SetRegistry(app.GetRegistry());
    editorLayer.SetConfiguration(config);
    editorLayer.Init();

    app.Run();

    editorLayer.Shutdown();
}

namespace {

int RunNew(int argc, char** argv) {
    Editor::ProjectRequest request;
    Editor::ScaffoldResult result;
    std::string flag;

    if (argc < 3) {
        log_error("Usage: RTypeEditor new <name> [--path <dir>] [--description <text>]");
        return 1;
    }
    request.name = argv[2];
    if (request.name.empty() || request.name.rfind("--", 0) == 0) {
        log_error("Usage: RTypeEditor new <name> [--path <dir>] [--description <text>]");
        return 1;
    }

    for (int index = 3; index < argc; ++index) {
        flag = argv[index];
        if (flag == "--path") {
            if (index + 1 >= argc) {
                log_error("Missing value for --path");
                return 1;
            }
            index += 1;
            request.path = argv[index];
        } else if (flag == "--description") {
            if (index + 1 >= argc) {
                log_error("Missing value for --description");
                return 1;
            }
            index += 1;
            request.description = argv[index];
        } else {
            log_error("Unknown option: " + flag);
            return 1;
        }
    }

    result = Editor::CreateProject(request);
    if (!result.ok) {
        return 1;
    }
    RunEditor(result.scenePath);
    return 0;
}

int RunCreate(int argc, char** argv) {
    Editor::ScaffoldResult result;
    std::string kind;
    std::string name;
    std::string enginePath;
    std::string flag;

    if (argc < 4) {
        log_error("Usage: RTypeEditor create <system|plugin> <Name> [--path <dir>]");
        return 1;
    }
    kind = argv[2];
    name = argv[3];
    if (name.empty() || name.rfind("--", 0) == 0) {
        log_error("Usage: RTypeEditor create <system|plugin> <Name> [--path <dir>]");
        return 1;
    }

    for (int index = 4; index < argc; ++index) {
        flag = argv[index];
        if (flag == "--path") {
            if (index + 1 >= argc) {
                log_error("Missing value for --path");
                return 1;
            }
            index += 1;
            enginePath = argv[index];
        } else {
            log_error("Unknown option: " + flag);
            return 1;
        }
    }

    if (kind == "system") {
        result = Editor::CreateSystem(name, enginePath);
    } else if (kind == "plugin") {
        result = Editor::CreatePlugin(name, enginePath);
    } else {
        log_error("Unknown create target: " + kind);
        return 1;
    }
    if (!result.ok) {
        return 1;
    }
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    std::string command;

    if (argc > 1) {
        command = argv[1];
        if (command == "new") {
            return RunNew(argc, argv);
        }
        if (command == "create") {
            return RunCreate(argc, argv);
        }
        log_error("Unknown command: " + command + ". Expected new or create");
        return 1;
    }

    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Raylib Gallery");
    SetTargetFPS(60);

    rlImGuiSetLoadFontsCallback(LoadGalleryFonts);

    rlImGuiBeginInitImGui();

    Editor::EditorStyle::Apply();
    Editor::Gallery gallery;
    std::string scenePath;

    rlImGuiEndInitImGui();

    while (!WindowShouldClose() && scenePath == "") {
        BeginDrawing();
        ClearBackground(DARKGRAY);

        rlImGuiBegin();

        scenePath = gallery.DrawGallery();

        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    if (scenePath != "") {
        RunEditor(scenePath);
    }
    return 0;
}
