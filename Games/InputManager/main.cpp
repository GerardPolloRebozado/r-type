#include "Core/Application.hpp"
#include "Plugins/PluginExample.hpp"
#include <iostream>

#include "Inputs/InputManager.hpp"
#include "Inputs/KeyboardController.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/TextPlugin.hpp"

int main() {
    std::cout << "======================================\n";
    std::cout << "      Starting Sandbox Game Demo      \n";
    std::cout << "======================================\n";

    Engine::ApplicationConfig config;
    config.windowTitle = "Example Sandbox Game";
    config.targetFPS = 60;

    Engine::Application app(config);

    Engine::InputManager input;

    auto& put = app.GetRegistry().AddResource<Engine::InputManager>();

    put.AddPlayer(1, std::make_unique<Engine::KeyboardController>());

    app.Run();

    return 0;
}
