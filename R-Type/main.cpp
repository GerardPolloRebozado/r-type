#include "Core/Application.hpp"
#include "SceneFiles.hpp"
#include <string>
#include "Plugins/AudioPlugin.hpp"
#include "Plugins/HierarchyPlugin.hpp"
#include "Plugins/InputPlugin.hpp"
#include "Plugins/MultiplayerPlugin.hpp"
#include "Plugins/ReplicationPlugin.hpp"
#include "Plugins/OpacityPlugin.hpp"
#include "Plugins/SignalPlugin.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/TextPlugin.hpp"
#include "Plugins/SpritePlugin.hpp"
#include "Plugins/ShapePlugin.hpp"
#include "Plugins/RotationPlugin.hpp"
#include "Plugins/ColliderPlugin.hpp"
#include "Plugins/CameraPlugin.hpp"
#include "Plugins/AnimationPlugin.hpp"
#include "Plugins/WavePlugin.hpp"
#include "Scripting/ScriptPlugin.hpp"
#include "UI/UiPlugin.hpp"
#include "Inputs/InputManager.hpp"
#include "Inputs/Controller.hpp"
#include <vector>
#include <cstdlib>
#include <raylib.h>

static void SetEnvVar(const char *name, const char *value) {
#ifdef _WIN32
    _putenv_s(name, value);
#else
    setenv(name, value, 1);
#endif
}

int main(int argc, char *argv[]) {
    ChangeDirectory(GetApplicationDirectory());
    Engine::ApplicationConfig config;
    config.windowTitle = "R-Type";
    config.windowWidth = 1920;
    config.windowHeight = 1080;
    config.virtualWidth = 1920;
    config.virtualHeight = 1080;
    config.targetFPS = 60;

    config.args = std::vector<std::string>(argv + 1, argv + argc);
    for (const auto& a : config.args) {
        if (a == "--headless") config.headless = true;
    }

    // Pass --host / --port to Lua via environment so the menu UI can
    // pre-fill connection fields and auto-connect.
    for (std::size_t i = 0; i + 1 < config.args.size(); ++i) {
        if (config.args[i] == "--host") {
            SetEnvVar("RP_TYPE_HOST", config.args[i + 1].c_str());
        }
        if (config.args[i] == "--port") {
            SetEnvVar("RP_TYPE_PORT", config.args[i + 1].c_str());
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
    Engine::AnimationPlugin animationPlugin;
    animationPlugin.Init(app.GetRegistry());
    Engine::WavePlugin wavePlugin;
    wavePlugin.Init(app.GetRegistry());
    Engine::ScriptPlugin scriptPlugin;
    scriptPlugin.Init(app.GetRegistry());
    Engine::UiPlugin().Init(app.GetRegistry());
    Engine::Multiplayer::ReplicationPlugin replicationPlugin;
    replicationPlugin.Init(app.GetRegistry());
    Engine::Multiplayer::MultiplayerPlugin multiplayerPlugin;
    multiplayerPlugin.Init(app.GetRegistry());

    auto& inputManager = app.GetRegistry().AddResource<Engine::InputManager>();
    auto controller = std::make_unique<Engine::Controller>();
    controller->addKeybind("UP", {87, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("UP", {265, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("DOWN", {83, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("DOWN", {264, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("LEFT", {65, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("LEFT", {263, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("RIGHT", {68, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("RIGHT", {262, 0, static_cast<Engine::DeviceType>(0)}, true);
    controller->addKeybind("SHOOT", {32, 0, static_cast<Engine::DeviceType>(0)}, true);
    inputManager.AddPlayer(1, std::move(controller));

    // Startup scene: a .scene argument wins (e.g. ./R-Type Level2.scene),
    // otherwise the project's startup scene.
    const std::string startupScene = Serialization::PickStartupScene(config.args, "menu.scene");
    Serialization::EnableSceneFiles(app.GetRegistry()).Load(startupScene);

    app.Run();
    return 0;
}
