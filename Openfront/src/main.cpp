#include <algorithm>
#include <chrono>
#include <string>
#include <thread>

#include "Common.hpp"
#include "NetworkManager.hpp"
#include "OpenfrontComponents.hpp"
#include "Renderer.hpp"
#include "Simulation.hpp"

#include "Assets/AssetManager.hpp"
#include "Core/Application.hpp"
#include "Inputs/Controller.hpp"
#include "Inputs/InputManager.hpp"
#include "Logging/Logger.hpp"
#include "Plugins/AudioPlugin.hpp"
#include "Plugins/InputPlugin.hpp"

// uses Engine::Application in headless mode
int RunDedicatedServer(uint16_t port, uint32_t seed) {
    Engine::ApplicationConfig cfg;
    cfg.headless = true;

    Engine::Application app(cfg);

    app.GetRegistry().RegisterComponent<Openfront::TacticalUnitComponent>();
    app.GetRegistry().RegisterComponent<Openfront::WarheadComponent>();
    app.GetRegistry().RegisterComponent<Openfront::FrontLineComponent>();

    Openfront::Simulation sim;
    Openfront::NetworkServer server;

    sim.SetEcsRegistry(&app.GetRegistry());
    sim.Init(6, 6, seed);
    server.SetMatchSettings(seed, 6, 6, sim.GetMatchDuration());

    if (!server.Start(port, 32)) {
        log_error("[Openfront] Failed to bind server on port " + std::to_string(port));
        return 1;
    }

    server.StartMatch(sim);
    log_info("[Openfront] Server listening on port " + std::to_string(port) +
             " (Seed: " + std::to_string(seed) + ")");

    auto lastTick = std::chrono::steady_clock::now();
    float accumulator = 0.0f;

    while (true) {
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - lastTick).count();
        lastTick = now;

        server.PollNetwork(sim, dt);

        accumulator += dt;
        while (accumulator >= Openfront::kSimulationTickDt) {
            server.TickTurn(sim);
            accumulator -= Openfront::kSimulationTickDt;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    server.Stop();
    return 0;
}

// uses Engine::Application with window + audio
int main(int argc, char* argv[]) {
    std::string runMode = "default";
    uint16_t port = Openfront::kDefaultPort;
    std::string joinIp = "127.0.0.1";
    std::string playerName = "Commander";
    uint32_t seed = 1337;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--server") {
            runMode = "server";
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                port = static_cast<uint16_t>(std::atoi(argv[++i]));
            }
        } else if (arg == "--host") {
            runMode = "host";
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                port = static_cast<uint16_t>(std::atoi(argv[++i]));
            }
        } else if (arg == "--join") {
            runMode = "join";
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                joinIp = argv[++i];
            }
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                port = static_cast<uint16_t>(std::atoi(argv[++i]));
            }
        } else if (arg == "--name" && i + 1 < argc) {
            playerName = argv[++i];
        } else if (arg == "--seed" && i + 1 < argc) {
            seed = static_cast<uint32_t>(std::atoi(argv[++i]));
        }
    }

    if (runMode == "server") {
        return RunDedicatedServer(port, seed);
    }

    Openfront::Simulation sim;
    Openfront::NetworkManager net;
    Openfront::Renderer renderer(1600, 900);

    Engine::ApplicationConfig cfg;
    cfg.windowWidth = 1600;
    cfg.windowHeight = 900;
    cfg.windowTitle = "OpenFront - C++ MMORTS Territory Strategy (Multiplayer)";
    cfg.targetFPS = 60;
    cfg.args = {argv + 1, argv + argc};

    // customRender is called each frame BEFORE BeginDrawing().
    // renderer.Render() manages its own BeginDrawing/EndDrawing pair, so it
    // must live here (not in onUI)
    cfg.customRender = [&](Engine::Registry& /*registry*/) {
        float frameTime = GetFrameTime();
        if (frameTime > 0.25f) {
            frameTime = 0.25f;
        }

        net.Update(sim, frameTime);
        sim.UpdateVisuals(frameTime);
        renderer.Update(sim, net, frameTime);
        renderer.Render(sim, net);
    };

    Engine::Application app(cfg);

    Engine::InputPlugin inputPlugin;
    inputPlugin.Init(app.GetRegistry());

    auto& inputManager = app.GetRegistry().AddResource<Engine::InputManager>();
    auto controller = std::make_unique<Engine::Controller>();
    controller->addKeybind("PAN_UP", {KEY_W, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("PAN_UP", {KEY_UP, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("PAN_DOWN", {KEY_S, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("PAN_DOWN", {KEY_DOWN, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("PAN_LEFT", {KEY_A, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("PAN_LEFT", {KEY_LEFT, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("PAN_RIGHT", {KEY_D, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("PAN_RIGHT", {KEY_RIGHT, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("ATTACK_10", {KEY_Z, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("ATTACK_20", {KEY_X, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("ATTACK_50", {KEY_C, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("ATTACK_100", {KEY_V, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("BUILD_CITY", {KEY_ONE, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("BUILD_PORT", {KEY_TWO, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("BUILD_FACTORY", {KEY_THREE, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("BUILD_DEFENSE", {KEY_FOUR, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("BUILD_SAM", {KEY_FIVE, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("BUILD_SILO", {KEY_SIX, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("LAUNCH_ATOM", {KEY_N, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("LAUNCH_H", {KEY_H, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("CANCEL", {KEY_ESCAPE, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("CONFIRM", {KEY_ENTER, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("CONFIRM", {KEY_SPACE, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("RESTART", {KEY_R, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("RESTART", {KEY_SPACE, 0, Engine::DeviceType::KEYBOARD}, false);
    controller->addKeybind("BACKSPACE", {KEY_BACKSPACE, 0, Engine::DeviceType::KEYBOARD}, false);
    inputManager.AddPlayer(1, std::move(controller));

    Engine::AudioPlugin audioPlugin;
    audioPlugin.Init(app.GetRegistry());

    auto& assetManager = app.GetRegistry().GetResource<Engine::AssetManager>();
    assetManager.LoadSound("explosion", "Openfront/assets/audio/explosion.wav");
    assetManager.LoadSound("capture", "Openfront/assets/audio/capture.wav");
    assetManager.LoadSound("nuke_launch", "Openfront/assets/audio/nuke_launch.ogg");
    assetManager.LoadSound("ui_click", "Openfront/assets/audio/ui_click.wav");

    renderer.SetAssetManager(&assetManager);
    renderer.SetController(inputManager.GetController(1));

    app.GetRegistry().RegisterComponent<Openfront::TacticalUnitComponent>();
    app.GetRegistry().RegisterComponent<Openfront::WarheadComponent>();
    app.GetRegistry().RegisterComponent<Openfront::FrontLineComponent>();

    sim.SetEcsRegistry(&app.GetRegistry());

    if (runMode == "host") {
        net.StartHost(sim, port, playerName);
        renderer.SetUIState(Openfront::UIState::HostLobby);
    } else if (runMode == "join") {
        net.StartClient(sim, joinIp, port, playerName);
        net.GetClient().SetOnGameStartedCallback([&renderer, &sim, &net]() {
            renderer.Init(sim, net.GetLocalPlayerId());
            renderer.SetUIState(Openfront::UIState::InGame);
        });
        renderer.SetUIState(Openfront::UIState::JoinLobby);
    } else {
        net.StartSinglePlayer(sim);
        renderer.SetUIState(Openfront::UIState::StartMenu);
    }

    renderer.Init(sim, net.GetLocalPlayerId());

    app.Run();

    net.Stop();
    return 0;
}
