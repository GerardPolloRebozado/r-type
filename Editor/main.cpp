#include "Core/Application.hpp"
#include "EditorLayer.hpp"
#include "Plugins/TransformPlugin.hpp"

int main() {
    Engine::ApplicationConfig config;
    config.windowTitle = "Engine Editor";
    config.windowWidth = 1280;
    config.windowHeight = 720;

    Editor::EditorLayer editorLayer;

    config.onUI = [&editorLayer]() { editorLayer.OnUpdate(); };

    Engine::Application app(config);

    // Initialize engine plugins (registers Transform component & system)
    Engine::TransformPlugin transformPlugin;
    transformPlugin.Init(app.GetRegistry());

    editorLayer.SetRegistry(&app.GetRegistry());
    editorLayer.Init();

    app.Run();

    editorLayer.Shutdown();

    return 0;
}
