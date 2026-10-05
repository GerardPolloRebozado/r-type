#include "Components/Position.hpp"
#include "Components/SpriteComponent.hpp"
#include "Core/Application.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/SpritePlugin.hpp"
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
    // Use ASSETS_PATH macro defined in CMake to always find the absolute path
    // regardless of where the executable is run from.
    try {
        Engine::SpritePlugin::AttachToEntity(app.GetRegistry(), myEntity,
                                             Engine::SpriteComponent{ASSETS_PATH "sprite.png"});
        std::cout << "Added SpriteComponent.\n";
    } catch (...) {
        std::cout << "Could not initialize SpriteComponent. Missing texture?\n";
    }

    app.Run();

    return 0;
}
