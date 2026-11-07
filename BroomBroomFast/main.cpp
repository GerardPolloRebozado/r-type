#include "Core/Application.hpp"
#include "Core/QuitRequest.hpp"
#include "Inputs/Controller.hpp"
#include "Inputs/InputManager.hpp"
#include "Plugins/AnimationPlugin.hpp"
#include "Plugins/AudioPlugin.hpp"
#include "Plugins/CameraPlugin.hpp"
#include "Plugins/ColliderPlugin.hpp"
#include "Plugins/HierarchyPlugin.hpp"
#include "Plugins/InputPlugin.hpp"
#include "Plugins/OpacityPlugin.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/RotationPlugin.hpp"
#include "Plugins/ShapePlugin.hpp"
#include "Plugins/SignalPlugin.hpp"
#include "Plugins/SpritePlugin.hpp"
#include "Plugins/TextPlugin.hpp"
#include "SceneFiles.hpp"
#include "Scripting/LuaBinder.hpp"
#include "Scripting/ScriptPlugin.hpp"
#include "UI/UiPlugin.hpp"
#include <raylib.h>
#include <sol/sol.hpp>
#include <string>
#include <vector>

namespace {

// Lets scripts close the game: Game.quit()
void RegisterGameApi(Engine::Registry& registry) {
    if (!registry.HasResource<Engine::LuaBinder>()) {
        registry.AddResource<Engine::LuaBinder>();
    }
    registry.GetResource<Engine::LuaBinder>().modules.push_back(
        [](sol::state_view lua, Engine::Registry& bound) {
            sol::table game = lua.create_named_table("Game");
            game["quit"] = [&bound]() {
                if (!bound.HasResource<Engine::QuitRequest>()) {
                    bound.AddResource<Engine::QuitRequest>();
                }
                bound.GetResource<Engine::QuitRequest>().requested = true;
            };
        });
}

void AddKeybinds(Engine::Registry& registry) {
    auto& inputManager = registry.AddResource<Engine::InputManager>();
    auto controller = std::make_unique<Engine::Controller>();
    const auto keyboard = static_cast<Engine::DeviceType>(0);
    controller->addKeybind("UP", {KEY_W, 0, keyboard}, false);
    controller->addKeybind("UP", {KEY_UP, 0, keyboard}, false);
    controller->addKeybind("DOWN", {KEY_S, 0, keyboard}, false);
    controller->addKeybind("DOWN", {KEY_DOWN, 0, keyboard}, false);
    controller->addKeybind("LEFT", {KEY_A, 0, keyboard}, false);
    controller->addKeybind("LEFT", {KEY_LEFT, 0, keyboard}, false);
    controller->addKeybind("RIGHT", {KEY_D, 0, keyboard}, false);
    controller->addKeybind("RIGHT", {KEY_RIGHT, 0, keyboard}, false);
    inputManager.AddPlayer(1, std::move(controller));
}

} // namespace

int main(int argc, char* argv[]) {
    ChangeDirectory(GetApplicationDirectory());
    Engine::ApplicationConfig config;
    config.windowTitle = "BroomBroomFast";
    config.windowWidth = 1920;
    config.windowHeight = 1080;
    config.virtualWidth = 1920;
    config.virtualHeight = 1080;
    config.targetFPS = 60;

    config.args = std::vector<std::string>(argv + 1, argv + argc);
    for (const auto& a : config.args) {
        if (a == "--headless") {
            config.headless = true;
        }
    }
    Engine::Application app(config);
    // ESC is used as "back" in the menus, so it must not close the window.
    if (!config.headless) {
        SetExitKey(KEY_NULL);
    }

    auto& registry = app.GetRegistry();
    // HierarchyPlugin before PositionPlugin: parent deltas must
    // propagate before prevX/prevY are snapshotted.
    Engine::HierarchyPlugin().Init(registry);
    Engine::PositionPlugin().Init(registry);
    Engine::TextPlugin().Init(registry);
    Engine::SpritePlugin().Init(registry);
    Engine::ShapePlugin().Init(registry);
    Engine::RotationPlugin().Init(registry);
    Engine::ColliderPlugin().Init(registry);
    Engine::OpacityPlugin().Init(registry);
    Engine::SignalPlugin().Init(registry);
    Engine::InputPlugin().Init(registry);
    Engine::AudioPlugin().Init(registry);
    Engine::CameraPlugin().Init(registry);
    Engine::AnimationPlugin().Init(registry);
    Engine::ScriptPlugin().Init(registry);
    Engine::UiPlugin().Init(registry);
    RegisterGameApi(registry);
    AddKeybinds(registry);

    // A .scene argument wins (e.g. ./BroomBroomFast Race_Dirt.scene),
    // otherwise the project's startup scene.
    const std::string startupScene = Serialization::PickStartupScene(config.args, "Menu.scene");
    Serialization::EnableSceneFiles(registry).Load(startupScene);

    app.Run();
    return 0;
}
