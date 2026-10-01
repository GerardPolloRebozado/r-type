#include "Core/Application.hpp"
#include "EditorLayer.hpp"
#include "Plugins/TransformPlugin.hpp"

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

    // Initialize engine plugins (registers Transform component & system)
    Engine::TransformPlugin transformPlugin;
    transformPlugin.Init(app.GetRegistry());

    editorLayer.SetRegistry(&app.GetRegistry());
    editorLayer.SetConfiguration(&config);
    editorLayer.Init();

    app.Run();

    editorLayer.Shutdown();

    return 0;
}
