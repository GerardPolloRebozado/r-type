#include "Core/Application.hpp"
#include "Plugins/PluginExample.hpp"
#include <iostream>

#include "Plugins/TextPlugin.hpp"
#include "Plugins/TransformPlugin.hpp"

int main() {
    std::cout << "======================================\n";
    std::cout << "      Starting Sandbox Game Demo      \n";
    std::cout << "======================================\n";

    Engine::ApplicationConfig config;
    config.windowTitle = "Example Sandbox Game";
    config.targetFPS = 60;

    Engine::Application app(config);

    Engine::TextPlugin textPlugin;
    Engine::TransformPlugin transform;

    transform.Init(app.GetRegistry());
    textPlugin.Init(app.GetRegistry());

    //create the entity for a text on the screen
    Engine::Entity mytextEntity = app.GetRegistry().CreateEntity();
    app.GetRegistry().AddComponent(mytextEntity, Engine::Position{100.0f, 100.0f});
    Engine::TextPlugin::AttachToEntity(app.GetRegistry(), mytextEntity, "Hello, World!");

    app.Run();

    return 0;
}
