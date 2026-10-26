#include "Core/Application.hpp"
#include "Inputs/Controller.hpp"
#include "Inputs/InputManager.hpp"
#include "Plugins/AudioPlugin.hpp"
#include "Plugins/CameraPlugin.hpp"
#include "Plugins/ColliderPlugin.hpp"
#include "Plugins/HierarchyPlugin.hpp"
#include "Plugins/InputPlugin.hpp"
#include "Plugins/MultiplayerPlugin.hpp"
#include "Plugins/OpacityPlugin.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/ReplicationPlugin.hpp"
#include "Plugins/RotationPlugin.hpp"
#include "Plugins/ShapePlugin.hpp"
#include "Plugins/SignalPlugin.hpp"
#include "Plugins/SpritePlugin.hpp"
#include "Plugins/TextPlugin.hpp"
#include "SceneFiles.hpp"
#include "Scripting/ScriptPlugin.hpp"
#include <string>
#include <raylib.h>
#include <vector>

int main(int argc, char* argv[]) {
    ChangeDirectory(GetApplicationDirectory());
    Engine::ApplicationConfig config;
    config.windowTitle = "Game";
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
    // HierarchyPlugin before PositionPlugin: parent deltas must
    // propagate before prevX/prevY are snapshotted.
    Engine::HierarchyPlugin hierarchyPlugin;
    hierarchyPlugin.Init(app.GetRegistry());
    Engine::PositionPlugin positionPlugin;
    positionPlugin.Init(app.GetRegistry());
    Engine::TextPlugin textPlugin;
    textPlugin.Init(app.GetRegistry());
    Engine::SpritePlugin spritePlugin;
    spritePlugin.Init(app.GetRegistry());
    Engine::ShapePlugin shapePlugin;
    shapePlugin.Init(app.GetRegistry());
    Engine::RotationPlugin rotationPlugin;
    rotationPlugin.Init(app.GetRegistry());
    Engine::ColliderPlugin colliderPlugin;
    colliderPlugin.Init(app.GetRegistry());
    Engine::OpacityPlugin opacityPlugin;
    opacityPlugin.Init(app.GetRegistry());
    Engine::SignalPlugin signalPlugin;
    signalPlugin.Init(app.GetRegistry());
    Engine::InputPlugin inputPlugin;
    inputPlugin.Init(app.GetRegistry());
    Engine::AudioPlugin audioPlugin;
    audioPlugin.Init(app.GetRegistry());
    Engine::CameraPlugin cameraPlugin;
    cameraPlugin.Init(app.GetRegistry());
    Engine::ScriptPlugin scriptPlugin;
    scriptPlugin.Init(app.GetRegistry());
    Engine::Multiplayer::ReplicationPlugin replicationPlugin;
    replicationPlugin.Init(app.GetRegistry());
    Engine::Multiplayer::MultiplayerPlugin multiplayerPlugin;
    multiplayerPlugin.Init(app.GetRegistry());

    auto& inputManager = app.GetRegistry().AddResource<Engine::InputManager>();
    auto controller = std::make_unique<Engine::Controller>();
    controller->addKeybind("SHOOT", {32, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("RIGHT", {68, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("RIGHT", {262, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("LEFT", {65, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("LEFT", {263, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("DOWN", {83, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("DOWN", {264, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("UP", {87, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("UP", {265, 0, static_cast<Engine::DeviceType>(0)}, true);
    inputManager.AddPlayer(1, std::move(controller));

    // Startup scene: a .scene argument wins (e.g. ./Game Level2.scene),
    // otherwise the project's startup scene.
    const std::string startupScene = Serialization::PickStartupScene(config.args, "test.scene");
    Serialization::EnableSceneFiles(app.GetRegistry()).Load(startupScene);

    app.Run();
    return 0;
}
