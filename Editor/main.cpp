#include "Core/Application.hpp"
#include "EditorLayer.hpp"
#include "Plugins/PositionPlugin.hpp"

int main() {
    Editor::EditorConfig config;
    config.windowTitle = "Engine Editor";
    config.m_ProjectFolder = "Games/EditorGame";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.maximized = true;

    Editor::EditorLayer editorLayer;

    config.onUI = [&editorLayer]() { editorLayer.OnUpdate(); };

    Engine::Application app(config);

    // Initialize engine plugins (registers Position component & system)
    Engine::PositionPlugin positionPlugin;
    positionPlugin.Init(app.GetRegistry());

    editorLayer.SetRegistry(&app.GetRegistry());
    editorLayer.SetConfiguration(&config);
    editorLayer.Init();

    app.Run();

    editorLayer.Shutdown();

    return 0;
}
