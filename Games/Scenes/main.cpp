// Scenes demo: a game made only in C++ (no editor, no .scene files) with
// three scenes: menu -> level -> win -> menu.
//
// Shows:
// - code scenes registered with SceneManager::Register,
// - switching with SceneManager::Load from a system,
// - a resource (Progress) that survives every switch, while entities don't.
//
// Controls: ENTER to start / continue, arrow keys to move, M back to menu.
#include "Components/ShapeComponent.hpp"
#include "Core/Application.hpp"
#include "ECS/Registry.hpp"
#include "ECS/System.hpp"
#include "Plugins/CameraPlugin.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/ShapePlugin.hpp"
#include "Plugins/TextPlugin.hpp"
#include "Resources/SceneManager.hpp"
#include <cmath>
#include <raylib.h>
#include <string>

namespace {

// Survives scene switches: resources are never cleared.
struct Progress {
    int wins = 0;
};

// Tags to find the level's entities. They are cleared with the scene, so no
// stale entity IDs are kept between switches.
struct PlayerTag {};
struct GoalTag {};

constexpr float kPlayerSpeed = 300.0f;
constexpr float kGoalReach = 45.0f;

void AddText(Engine::Registry& registry, const std::string& text, float size, float x, float y) {
    const Engine::Entity entity = registry.CreateEntity();
    Engine::TextPlugin::AttachToEntity(registry, entity, text, size, Engine::Position(x, y));
}

void AddShape(Engine::Registry& registry, Engine::Entity entity, Engine::ShapeType type,
              Color color, float x, float y) {
    registry.RequireComponent<Engine::Position>(entity) = Engine::Position(x, y);
    Engine::ShapeComponent shape;
    shape.type = type;
    shape.width = 50.0f;
    shape.height = 50.0f;
    shape.radius = 25.0f;
    shape.color = color;
    Engine::ShapePlugin::AttachToEntity(registry, entity, shape);
}

// --- Scenes: each one only creates its entities. ---

void BuildMenu(Engine::Registry& registry) {
    const int wins = registry.GetResource<Progress>().wins;
    AddText(registry, "SCENES DEMO", 60.0f, 80.0f, 120.0f);
    AddText(registry, "Press ENTER to play", 30.0f, 80.0f, 240.0f);
    AddText(registry, "Wins so far: " + std::to_string(wins), 24.0f, 80.0f, 300.0f);
}

void BuildLevel(Engine::Registry& registry) {
    AddText(registry, "Reach the green circle", 28.0f, 40.0f, 30.0f);
    AddText(registry, "Arrows: move    M: back to menu", 20.0f, 40.0f, 70.0f);

    const Engine::Entity player = registry.CreateEntity();
    AddShape(registry, player, Engine::ShapeType::RECTANGLE, SKYBLUE, 120.0f, 360.0f);
    registry.AddComponent(player, PlayerTag{});

    const Engine::Entity goal = registry.CreateEntity();
    AddShape(registry, goal, Engine::ShapeType::CIRCLE, GREEN, 1000.0f, 360.0f);
    registry.AddComponent(goal, GoalTag{});
}

void BuildWin(Engine::Registry& registry) {
    AddText(registry, "YOU WIN!", 60.0f, 80.0f, 120.0f);
    AddText(registry, "Press ENTER for the menu", 30.0f, 80.0f, 240.0f);
}

// --- Game logic: decides when to switch scenes. ---

class SceneFlowSystem : public Engine::System {
  public:
    void Update(Engine::Registry& registry, float dt) override {
        auto& scenes = registry.GetResource<Engine::SceneManager>();
        const std::string& current = scenes.Current();

        if (current == "menu" && IsKeyPressed(KEY_ENTER)) {
            scenes.Load("level");
        } else if (current == "win" && IsKeyPressed(KEY_ENTER)) {
            scenes.Load("menu");
        } else if (current == "level") {
            UpdateLevel(registry, scenes, dt);
        }
    }

  private:
    static void UpdateLevel(Engine::Registry& registry, Engine::SceneManager& scenes, float dt) {
        if (IsKeyPressed(KEY_M)) {
            scenes.Load("menu");
            return;
        }
        auto players = registry.GetComponentArray<PlayerTag>();
        auto goals = registry.GetComponentArray<GoalTag>();
        if (players->size() == 0 || goals->size() == 0) {
            return;
        }
        const Engine::Entity player = players->begin()->first;
        const Engine::Entity goal = goals->begin()->first;

        auto& position = registry.RequireComponent<Engine::Position>(player);
        const float dx = static_cast<float>(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT));
        const float dy = static_cast<float>(IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP));
        position.x += dx * kPlayerSpeed * dt;
        position.y += dy * kPlayerSpeed * dt;

        const auto& target = registry.RequireComponent<Engine::Position>(goal);
        if (std::hypot(position.x - target.x, position.y - target.y) < kGoalReach) {
            registry.GetResource<Progress>().wins += 1;
            scenes.Load("win");
        }
    }
};

} // namespace

int main() {
    Engine::ApplicationConfig config;
    config.windowTitle = "Scenes Demo";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.targetFPS = 60;

    Engine::Application app(config);
    auto& registry = app.GetRegistry();

    Engine::PositionPlugin().Init(registry);
    Engine::TextPlugin().Init(registry);
    Engine::ShapePlugin().Init(registry);
    Engine::CameraPlugin().Init(registry);
    registry.RegisterComponent<PlayerTag>();
    registry.RegisterComponent<GoalTag>();
    registry.RegisterSystem<SceneFlowSystem>();

    registry.AddResource<Progress>();

    auto& scenes = registry.AddResource<Engine::SceneManager>();
    scenes.Register("menu", BuildMenu);
    scenes.Register("level", BuildLevel);
    scenes.Register("win", BuildWin);
    scenes.Load("menu");

    app.Run();
    return 0;
}
