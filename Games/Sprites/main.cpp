#include "Core/Application.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/SpritePlugin.hpp"
#include "Components/Position.hpp"
#include "Components/SpriteComponent.hpp"
#include <iostream>

int main() {
    Engine::ApplicationConfig config;
    config.windowTitle = "Sprite Component Demo";
    config.targetFPS = 60;

    Engine::Application app(config);

    // Initialize plugins
    Engine::PositionPlugin positionPlugin;
    positionPlugin.Init(app.GetRegistry());

    Engine::SpritePlugin spritePlugin;
    spritePlugin.Init(app.GetRegistry());

    // Create an entity
    Engine::Entity myEntity = app.GetRegistry().CreateEntity();
    std::cout << "Spawned entity: " << myEntity << "\n";

    // Add position (position 200, 200)
    app.GetRegistry().AddComponent(myEntity, Engine::Position{200.0f, 200.0f});

    // Add sprite component
    // Assuming there is an image named 'sprite.png' in the working directory
    // If there isn't, Raylib will output a warning but won't crash
    try {
        app.GetRegistry().AddComponent(myEntity, Engine::SpriteComponent{"sprite.png"});
        std::cout << "Added SpriteComponent.\n";
    } catch (...) {
        std::cout << "Could not initialize SpriteComponent. Missing texture?\n";
    }

    app.Run();

    return 0;
}
