#include "Components/Position.hpp"
#include "Components/SpriteComponent.hpp"
#include "Core/Application.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/SpritePlugin.hpp"
#include <exception>
#include <iostream>

#include "Assets/AssetManager.hpp"

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
    auto& assets = app.GetRegistry().GetResource<Engine::AssetManager>();

    try {
        Engine::SpritePlugin::AttachToEntity(
            app.GetRegistry(), myEntity, Engine::SpriteComponent{assets.GetSprite("sprite.png")});
        std::cout << "Added SpriteComponent.\n";
    } catch (const std::exception& e) {
        std::cerr << "Could not initialize SpriteComponent: " << e.what() << "\n";
    }

    app.Run();

    return 0;
}
