#include "BundledAssets.hpp"
#include "Core/Application.hpp"
#include "EditorLayer.hpp"
#include "EditorStyle.hpp"
#include "Gallery.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/SpritePlugin.hpp"
#include "Plugins/TextPlugin.hpp"
#include "imgui.h"
#include "raylib.hpp"
#include "rlImGui.h"
#include <extras/FA6FreeSolidFontData.h>
#include <extras/IconsFontAwesome6.h>
#include <filesystem>

using std::string;

void RunEditor(string scenePathString) {
    Editor::EditorConfig config;

    std::filesystem::path projectFolder = scenePathString;
    projectFolder = projectFolder.parent_path();

    config.windowTitle = "Engine Editor";
    config.m_ProjectFolder = projectFolder;
    config.m_InitialScenePath = scenePathString;
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
    Engine::PositionPlugin positionPlugin;
    positionPlugin.Init(app.GetRegistry());
    Engine::TextPlugin text_plugin;
    text_plugin.Init(app.GetRegistry());
    Engine::SpritePlugin spritePlugin;
    spritePlugin.Init(app.GetRegistry());

    editorLayer.SetRegistry(app.GetRegistry());
    editorLayer.SetConfiguration(config);
    editorLayer.Init();

    app.Run();

    editorLayer.Shutdown();
}

int main() {
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
