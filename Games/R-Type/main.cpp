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
#include "SceneSerializer.hpp"
#include "Scripting/ScriptPlugin.hpp"
#include <optional>
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
    // ReplicationPlugin before MultiplayerPlugin: systems run in registration
    // order, so ReplicationSystem installs its packet handlers before
    // MultiplayerSystem polls the network.
    Engine::Multiplayer::ReplicationPlugin replicationPlugin;
    replicationPlugin.Init(app.GetRegistry());
    Engine::Multiplayer::MultiplayerPlugin multiplayerPlugin;
    multiplayerPlugin.Init(app.GetRegistry());

    auto& inputManager = app.GetRegistry().AddResource<Engine::InputManager>();
    auto controller = std::make_unique<Engine::Controller>();
    controller->addKeybind("UP", {87, 0, static_cast<Engine::DeviceType>(0)});
    controller->addKeybind("UP", {265, 0, static_cast<Engine::DeviceType>(0)});
    controller->addKeybind("DOWN", {83, 0, static_cast<Engine::DeviceType>(0)});
    controller->addKeybind("DOWN", {264, 0, static_cast<Engine::DeviceType>(0)});
    controller->addKeybind("LEFT", {65, 0, static_cast<Engine::DeviceType>(0)});
    controller->addKeybind("LEFT", {263, 0, static_cast<Engine::DeviceType>(0)});
    controller->addKeybind("RIGHT", {68, 0, static_cast<Engine::DeviceType>(0)});
    controller->addKeybind("RIGHT", {262, 0, static_cast<Engine::DeviceType>(0)});
    controller->addKeybind("SHOOT", {32, 0, static_cast<Engine::DeviceType>(0)});
    inputManager.AddPlayer(1, std::move(controller));

    Serialization::SceneSerializer serializer;
    serializer.Deserialize("test.scene", app.GetRegistry(), std::nullopt);

    app.Run();
    return 0;
}