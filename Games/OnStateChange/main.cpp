#include "Core/Application.hpp"
#include <iostream>
#include <ranges>

#include "Components/Position.hpp"
#include "Plugins/TextPlugin.hpp"
#include "Systems/StatesSystem.hpp"

enum class GameState { MainMenu, Game };

class KeySystem : public Engine::System {
  public:
    ~KeySystem() noexcept override = default;

    void Update(Engine::Registry& registry, float dt) override {
        if (raylib::Keyboard::GetKeyPressed() == KEY_G) {
            std::cout << "Pressed G" << std::endl;
            registry.GetResource<Engine::NextState<GameState>>().set(GameState::Game);
        }
    }
};

struct MainMenuEntity {};

class MainMenuCleanupSystem : public Engine::System {
  public:
    void Update(Engine::Registry& registry, float dt) override {
        // Ensure the event resource exists
        if (!registry.HasResource<Engine::StateTransitionEvent<GameState>>()) {
            return;
        }

        const auto& event = registry.GetResource<Engine::StateTransitionEvent<GameState>>();
        // Check if we JUST exited the MainMenu state this frame
        if (event.exited.has_value() && event.exited.value() == GameState::MainMenu) {
            // Find all entities tagged as MainMenuEntity and destroy them
            const auto view = registry.GetComponentArray<MainMenuEntity>();
            for (const auto key : *view | std::views::keys) {
                registry.QueueDestroyEntity(key);
            }
        }
    }
};

int main() {
    std::cout << "======================================\n";
    std::cout << "               State Sandbox          \n";
    std::cout << "======================================\n";

    Engine::ApplicationConfig config;
    config.windowTitle = "State Sandbox";
    config.targetFPS = 60;
    ChangeDirectory(GetApplicationDirectory());

    Engine::Application app(config);
    app.GetRegistry().RegisterComponent<MainMenuEntity>();
    app.GetRegistry().AddResource<Engine::State<GameState>>(
        Engine::State<GameState>(GameState::MainMenu));
    app.GetRegistry().AddResource<Engine::NextState<GameState>>(Engine::NextState<GameState>());
    app.GetRegistry().AddResource<Engine::StateTransitionEvent<GameState>>();
    app.GetRegistry().RegisterSystem<Engine::StatesSystem<GameState>>();
    app.GetRegistry().RegisterSystem<KeySystem>();
    app.GetRegistry().RegisterSystem<MainMenuCleanupSystem>();
    app.GetRegistry().RegisterComponent<Engine::Position>();
    const auto menuText = app.GetRegistry().CreateEntity();
    Engine::TextPlugin text_plugin;
    text_plugin.Init(app.GetRegistry());
    Engine::TextPlugin::AttachToEntity(app.GetRegistry(), menuText,
                                       "This text will disapear when you press G", 50.0,
                                       Engine::Position{960.0f, 500.0f});
    app.GetRegistry().AddComponent(menuText, MainMenuEntity{});

    app.Run();

    return 0;
}
