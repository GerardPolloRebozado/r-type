#include "Core/Application.hpp"
#include "EditorLayer.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/SpritePlugin.hpp"
#include "Plugins/TextPlugin.hpp"

int main() {
    Editor::EditorConfig config;
    config.windowTitle = "Engine Editor";
    config.m_ProjectFolder = "Games/EditorGame";
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

    return 0;
}
