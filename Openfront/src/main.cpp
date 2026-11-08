#include <algorithm>
#include <chrono>
#include <string>
#include <thread>

#include "Common.hpp"
#include "MultiRoomServer.hpp"
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
#include "Plugins/CameraPlugin.hpp"
#include "Plugins/InputPlugin.hpp"

#include <sol/sol.hpp>

int RunMultiRoomServer(uint16_t basePort, const std::vector<Openfront::RoomConfig>& configs) {
    Openfront::MultiRoomServer server;
    if (!server.Start(configs)) {
        log_error("[Openfront] One or more rooms failed to bind");
        return 1;
    }

    log_info("[Openfront] Dedicated multi-room server: " + std::to_string(configs.size()) +
             " room(s), ports " + std::to_string(basePort) + ".." +
             std::to_string(basePort + configs.size() - 1));

    server.Run();
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
    Openfront::MapType mapType = Openfront::MapType::Procedural;

    int roomCount = 0;
    std::vector<uint16_t> roomCaps;
    std::vector<std::string> roomMaps;
    std::vector<std::string> roomNames;
    std::vector<int> roomBots;

    auto splitCsvInts = [](const std::string& s) {
        std::vector<int> out;
        std::string cur;
        for (char c : s) {
            if (c == ',') {
                if (!cur.empty()) {
                    out.push_back(std::atoi(cur.c_str()));
                    cur.clear();
                }
            } else {
                cur += c;
            }
        }
        if (!cur.empty()) {
            out.push_back(std::atoi(cur.c_str()));
        }
        return out;
    };
    auto splitCsvStrings = [](const std::string& s) {
        std::vector<std::string> out;
        std::string cur;
        for (char c : s) {
            if (c == ',') {
                if (!cur.empty()) {
                    out.push_back(cur);
                    cur.clear();
                }
            } else {
                cur += c;
            }
        }
        if (!cur.empty()) {
            out.push_back(cur);
        }
        return out;
    };

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--server") {
            runMode = "server";
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                port = static_cast<uint16_t>(std::atoi(argv[++i]));
            }
        } else if (arg == "--rooms" && i + 1 < argc) {
            roomCount = std::atoi(argv[++i]);
        } else if (arg == "--room-cap" && i + 1 < argc) {
            for (int v : splitCsvInts(argv[++i])) {
                roomCaps.push_back(static_cast<uint16_t>(std::max(1, v)));
            }
        } else if (arg == "--room-maps" && i + 1 < argc) {
            roomMaps = splitCsvStrings(argv[++i]);
        } else if (arg == "--room-name" && i + 1 < argc) {
            roomNames = splitCsvStrings(argv[++i]);
        } else if (arg == "--room-bots" && i + 1 < argc) {
            roomBots = splitCsvInts(argv[++i]);
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
        } else if (arg == "--map" && i + 1 < argc) {
            std::string mapName = argv[++i];
            if (mapName == "europe") {
                mapType = Openfront::MapType::Europe;
            } else if (mapName == "world") {
                mapType = Openfront::MapType::World;
            } else if (mapName == "pangaea") {
                mapType = Openfront::MapType::Pangaea;
            } else if (mapName == "archipelago") {
                mapType = Openfront::MapType::Archipelago;
            }
        }
    }

    if (runMode == "server") {
        if (roomCount < 1) {
            roomCount = 1;
        }
        std::vector<Openfront::RoomConfig> configs;
        configs.reserve(roomCount);
        for (int r = 0; r < roomCount; ++r) {
            Openfront::RoomConfig cfg;
            cfg.name = roomNames.empty() ? ("Room " + std::to_string(r + 1))
                                         : roomNames[static_cast<size_t>(r) % roomNames.size()];
            ;
            cfg.port = static_cast<uint16_t>(port + r);
            size_t ci = roomCaps.empty()        ? 8
                        : (r < roomCaps.size()) ? static_cast<size_t>(r)
                                                : roomCaps.size() - 1;
            cfg.maxPlayers = roomCaps.empty() ? 8 : roomCaps[ci];
            size_t mi = roomMaps.empty()        ? 0
                        : (r < roomMaps.size()) ? static_cast<size_t>(r)
                                                : roomMaps.size() - 1;
            if (!roomMaps.empty()) {
                Openfront::ParseMapTypeName(roomMaps[mi], cfg.mapType);
            }
            size_t bi = roomBots.empty()        ? 0
                        : (r < roomBots.size()) ? static_cast<size_t>(r)
                                                : roomBots.size() - 1;
            if (!roomBots.empty()) {
                cfg.bots = static_cast<uint8_t>(std::clamp(roomBots[bi], 0, 6));
            }
            cfg.seed = seed;
            configs.push_back(cfg);
        }
        return RunMultiRoomServer(port, configs);
    }

    Openfront::Simulation sim;
    Openfront::NetworkManager net;
    Openfront::Renderer renderer(1600, 900);

    // -----------------------------------------------------------------------
    // Lua scripting (engine scripting system demonstration)
    // -----------------------------------------------------------------------
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string);
    try {
        lua.script_file("Openfront/scripts/game_rules.lua");
        log_info("[Openfront] Loaded game_rules.lua");
        // Demonstrate: read a building cost from Lua
        sol::function getCost = lua["get_building_cost"];
        if (getCost) {
            int cityCost = getCost("City");
            log_info("[Openfront] Lua: City building cost = " + std::to_string(cityCost));
        }
    } catch (const std::exception& e) {
        log_error("[Openfront] Failed to load game_rules.lua: " + std::string(e.what()));
    }

    Engine::ApplicationConfig cfg;
    cfg.customRenderHandlesDrawing = true;
    cfg.windowWidth = 1600;
    cfg.windowHeight = 900;
    cfg.windowTitle = "OpenFront - C++ MMORTS Territory Strategy (Multiplayer)";
    cfg.targetFPS = 60;
    cfg.args = {argv + 1, argv + argc};

    // customRender is called each frame BEFORE BeginDrawing().
    // renderer.Render() manages its own BeginDrawing/EndDrawing pair, so it
    // must live here (not in onUI)
    Engine::AssetManager* assetManagerPtr = nullptr;
    cfg.customRender = [&](Engine::Registry& /*registry*/) {
        float frameTime = GetFrameTime();
        if (frameTime > 0.25f) {
            frameTime = 0.25f;
        }

        // Update background music
        if (assetManagerPtr) {
            auto music = assetManagerPtr->GetMusic("ambient");
            if (music && !music->IsPlaying()) {
                music->Play();
            }
            if (music) {
                music->Update();
            }
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
    assetManagerPtr = &assetManager;
    assetManager.LoadSound("explosion", "Openfront/assets/audio/explosion.wav");
    assetManager.LoadSound("capture", "Openfront/assets/audio/capture.wav");
    assetManager.LoadSound("nuke_launch", "Openfront/assets/audio/nuke_launch.ogg");
    assetManager.LoadSound("ui_click", "Openfront/assets/audio/ui_click.wav");
    assetManager.LoadSound("build", "Openfront/assets/audio/build.wav");
    assetManager.LoadSound("victory", "Openfront/assets/audio/victory.wav");
    assetManager.LoadSound("defeat", "Openfront/assets/audio/defeat.wav");
    assetManager.LoadSound("ui_hover", "Openfront/assets/audio/ui_hover.wav");
    assetManager.LoadSound("troop_move", "Openfront/assets/audio/troop_move.wav");
    assetManager.LoadSound("capture2", "Openfront/assets/audio/capture2.wav");
    assetManager.LoadSound("impact", "Openfront/assets/audio/impact.wav");
    assetManager.LoadMusic("ambient", "Openfront/assets/audio/ambient.wav", true);

    renderer.SetAssetManager(&assetManager);
    renderer.SetController(inputManager.GetController(1));

    app.GetRegistry().RegisterComponent<Openfront::TacticalUnitComponent>();
    app.GetRegistry().RegisterComponent<Openfront::WarheadComponent>();
    app.GetRegistry().RegisterComponent<Openfront::FrontLineComponent>();

    // -----------------------------------------------------------------------
    // Camera – use engine's CameraPlugin (RTS pan/zoom synced to engine ECS)
    // -----------------------------------------------------------------------
    Engine::CameraPlugin cameraPlugin;
    cameraPlugin.Init(app.GetRegistry());

    Engine::Entity cameraEntity = app.GetRegistry().CreateEntity();
    app.GetRegistry().AddComponent<Engine::Position>(cameraEntity, Engine::Position{0.0f, 0.0f, 0});
    app.GetRegistry().AddComponent<Engine::CameraComponent>(cameraEntity,
                                                            Engine::CameraComponent{1.0f, true});
    renderer.SetCameraEntity(cameraEntity);
    renderer.SetRegistry(&app.GetRegistry());

    sim.SetEcsRegistry(&app.GetRegistry());

    if (runMode == "host") {
        net.StartHost(sim, port, playerName, mapType);
        renderer.SetUIState(Openfront::UIState::HostLobby);
    } else if (runMode == "join") {
        net.StartClient(sim, joinIp, port, playerName);
        net.GetClient().SetOnGameStartedCallback([&renderer, &sim, &net]() {
            renderer.Init(sim, net.GetLocalPlayerId());
            renderer.SetUIState(Openfront::UIState::InGame);
        });
        renderer.SetUIState(Openfront::UIState::JoinLobby);
    } else {
        net.StartSinglePlayer(sim, mapType);
        renderer.SetUIState(Openfront::UIState::StartMenu);
    }

    renderer.Init(sim, net.GetLocalPlayerId());

    renderer.SetSelectedMapType(mapType);

    app.Run();
    net.Stop();
    return 0;
}
