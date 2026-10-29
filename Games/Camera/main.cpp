#include "Components/CameraComponent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ScreenSpace.hpp"
#include "Components/ShapeComponent.hpp"
#include "Core/Application.hpp"
#include "Plugins/CameraPlugin.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/ShapePlugin.hpp"
#include "Plugins/TextPlugin.hpp"
#include <algorithm>
#include <cmath>
#include <string>

namespace {
constexpr float WORLD_WIDTH = 3000.0f;
constexpr float WORLD_HEIGHT = 2000.0f;

Engine::Entity SpawnShape(Engine::Registry& registry, float x, float y,
                          const Engine::ShapeComponent& shape) {
    Engine::Entity entity = registry.CreateEntity();
    registry.RequireComponent<Engine::Position>(entity) = Engine::Position{x, y};
    Engine::ShapePlugin::AttachToEntity(registry, entity, shape);
    return entity;
}

Engine::Entity SpawnText(Engine::Registry& registry, float x, float y, const std::string& text,
                         float fontSize) {
    Engine::Entity entity = registry.CreateEntity();
    Engine::TextPlugin::AttachToEntity(registry, entity, text, fontSize);
    registry.RequireComponent<Engine::Position>(entity) = Engine::Position{x, y};
    return entity;
}
} // namespace

// Arrows: move the player. +/- or mouse wheel: zoom. Q/E: rotate the camera. Space: shake.
// Left click: drop a marker where the mouse is in the world (screen -> world conversion).
class CameraDemoSystem : public Engine::System {
  public:
    Engine::Entity player = 0;
    Engine::Entity camera = 0;

    void Update(Engine::Registry& registry, float dt) override {
        auto& position = registry.RequireComponent<Engine::Position>(player);
        const float speed = 500.0f * dt;
        if (IsKeyDown(KEY_RIGHT)) {
            position.x += speed;
        }
        if (IsKeyDown(KEY_LEFT)) {
            position.x -= speed;
        }
        if (IsKeyDown(KEY_DOWN)) {
            position.y += speed;
        }
        if (IsKeyDown(KEY_UP)) {
            position.y -= speed;
        }
        position.x = std::clamp(position.x, 0.0f, WORLD_WIDTH);
        position.y = std::clamp(position.y, 0.0f, WORLD_HEIGHT);

        auto& cam = registry.RequireComponent<Engine::CameraComponent>(camera);
        // Zoom with typed '+'/'-' characters (works on any keyboard layout, unlike key codes)
        // and with the mouse wheel. Holding a key repeats the character.
        float zoomSteps = GetMouseWheelMove();
        for (int c = GetCharPressed(); c != 0; c = GetCharPressed()) {
            if (c == '+') {
                zoomSteps += 1.0f;
            } else if (c == '-') {
                zoomSteps -= 1.0f;
            }
        }
        if (zoomSteps != 0.0f) {
            cam.zoom = std::clamp(cam.zoom * std::pow(1.1f, zoomSteps), 0.25f, 4.0f);
        }
        // The camera rotation is the Rotation of its entity
        auto& rotation = registry.RequireComponent<Engine::Rotation>(camera);
        if (IsKeyDown(KEY_Q)) {
            rotation.angle -= 45.0f * dt;
        }
        if (IsKeyDown(KEY_E)) {
            rotation.angle += 45.0f * dt;
        }
        if (IsKeyPressed(KEY_SPACE)) {
            cam.shakeTrauma = 1.0f;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            const Vector2 world = Engine::CameraSystem::ScreenToWorld(registry, GetMousePosition());
            Engine::ShapeComponent marker;
            marker.type = Engine::ShapeType::CIRCLE;
            marker.radius = 8.0f;
            marker.color = YELLOW;
            SpawnShape(registry, world.x, world.y, marker);
        }
    }
};

int main() {
    Engine::ApplicationConfig config;
    config.windowTitle = "Camera Demo";
    config.targetFPS = 60;

    Engine::Application app(config);
    Engine::Registry& registry = app.GetRegistry();

    Engine::PositionPlugin positionPlugin;
    positionPlugin.Init(registry);
    Engine::ShapePlugin shapePlugin;
    shapePlugin.Init(registry);
    Engine::TextPlugin textPlugin;
    textPlugin.Init(registry);

    // World: a border and a grid of labelled tiles, much bigger than the window
    Engine::ShapeComponent border;
    border.width = WORLD_WIDTH;
    border.height = WORLD_HEIGHT;
    border.filled = false;
    border.thickness = 6.0f;
    border.color = RED;
    SpawnShape(registry, 0.0f, 0.0f, border);

    for (float x = 0.0f; x < WORLD_WIDTH; x += 250.0f) {
        for (float y = 0.0f; y < WORLD_HEIGHT; y += 250.0f) {
            Engine::ShapeComponent tile;
            tile.width = 230.0f;
            tile.height = 230.0f;
            tile.filled = false;
            tile.thickness = 2.0f;
            tile.color = DARKGRAY;
            SpawnShape(registry, x + 10.0f, y + 10.0f, tile);
            SpawnText(registry, x + 20.0f, y + 20.0f,
                      std::to_string(static_cast<int>(x)) + ", " +
                          std::to_string(static_cast<int>(y)),
                      20.0f);
        }
    }

    Engine::ShapeComponent playerShape;
    playerShape.type = Engine::ShapeType::CIRCLE;
    playerShape.radius = 20.0f;
    playerShape.color = SKYBLUE;
    Engine::Entity player = SpawnShape(registry, WORLD_WIDTH / 2, WORLD_HEIGHT / 2, playerShape);

    // Camera plugin is initialised after the gameplay systems so it follows this frame's movement
    auto demo = registry.RegisterSystem<CameraDemoSystem>();
    Engine::CameraPlugin cameraPlugin;
    cameraPlugin.Init(registry);

    Engine::Entity camera = registry.CreateEntity();
    Engine::CameraComponent cameraComponent;
    cameraComponent.followTarget = player;
    cameraComponent.followSpeed = 5.0f;
    cameraComponent.clampToBounds = true;
    cameraComponent.bounds = {0.0f, 0.0f, WORLD_WIDTH, WORLD_HEIGHT};
    cameraComponent.shakeMagnitude = 20.0f;
    Engine::CameraPlugin::AttachToEntity(registry, camera, cameraComponent);

    demo->player = player;
    demo->camera = camera;

    // HUD: tagged ScreenSpace, so it stays in the corner whatever the camera does
    Engine::ShapeComponent hudPanel;
    hudPanel.width = 640.0f;
    hudPanel.height = 70.0f;
    hudPanel.color = Color{0, 0, 0, 180};
    Engine::Entity panel = SpawnShape(registry, 10.0f, 10.0f, hudPanel);
    registry.AddComponent(panel, Engine::ScreenSpace{});

    Engine::Entity help = SpawnText(registry, 20.0f, 20.0f,
                                    "Arrows: move   +/- or wheel: zoom   Q/E: rotate   Space: shake",
                                    20.0f);
    registry.AddComponent(help, Engine::ScreenSpace{});
    Engine::Entity help2 = SpawnText(registry, 20.0f, 48.0f, "Click: drop a marker in the world", 20.0f);
    registry.AddComponent(help2, Engine::ScreenSpace{});

    app.Run();

    return 0;
}
