#include "Core/Application.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/TextPlugin.hpp"
#include "Serialization/SceneSerializer.hpp"

int main() {
    Engine::ApplicationConfig config;
    config.windowTitle = "Game";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.targetFPS = 60;

    Engine::Application app(config);
    Engine::PositionPlugin positionPlugin;
    positionPlugin.Init(app.GetRegistry());
    Engine::TextPlugin textPlugin;
    textPlugin.Init(app.GetRegistry());

    Editor::SceneSerializer serializer;
    serializer.Deserialize("test.scene", app.GetRegistry(), nullptr);

    app.Run();
    return 0;
}
