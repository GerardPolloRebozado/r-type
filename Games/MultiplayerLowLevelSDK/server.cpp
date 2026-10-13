#include "Components/Position.hpp"
#include "Core/Application.hpp"
#include "ENetContext.hpp"
#include "Inputs/InputManager.hpp"
#include "PacketTypes.hpp"
#include "Plugins/ColliderPlugin.hpp"
#include "Plugins/InputPlugin.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/ShapePlugin.hpp"
#include "Plugins/TextPlugin.hpp"
#include "Resources/NetworkHostResource.hpp"
#include "Systems/MultiplayerSystem.hpp"
#include <iostream>
#include <raylib.h>

static void KeyPressed(Engine::Registry& registry, ENetPeer* peer, const KeyPress& keyPress) {
    const auto entity = Engine::Multiplayer::NetworkHostResource::GetEntityID(peer);
    if (!entity.has_value()) {
        return;
    }
    auto& position = registry.GetComponent<Engine::Position>(entity.value());
    switch (keyPress.key) {
        case 'w':
            position.moved = true;
            position.y -= 1;
            break;
        case 'a':
            position.moved = true;
            position.x -= 1;
            break;
        case 's':
            position.moved = true;
            position.y += 1;
            break;
        case 'd':
            position.moved = true;
            position.x += 1;
            break;
        default:
            return;
    }
    const auto newPosition = NewPlayerPos{
        registry.GetComponent<Engine::Multiplayer::PlayerInformation>(entity.value()).id, position};
    const auto newPosNotification = Engine::Multiplayer::NetworkHostResource::CreatePacket(
        NewPlayerPosPacket, newPosition, false);
    registry.GetResource<Engine::Multiplayer::NetworkHostResource>().SendAllHost(newPosNotification,
                                                                                 0);
}

static void OnConnect(Engine::Registry& registry, ENetPeer* peer) {
    const auto playerEntity = Engine::Multiplayer::NetworkHostResource::GetEntityID(peer).value();
    constexpr Engine::ShapeComponent shape;
    registry.AddComponent(playerEntity, shape);

    const auto newPlayer =
        registry.GetComponent<Engine::Multiplayer::PlayerInformation>(playerEntity);

    const auto& newPlayerPos = registry.GetComponent<Engine::Position>(playerEntity);
    auto playerArray = registry.GetComponentArray<Engine::Multiplayer::PlayerInformation>();
    for (const auto& [existingEntity, existingPlayer] : *playerArray) {
        if (existingPlayer.id == newPlayer.id) {
            continue;
        }
        // send all other player positions
        auto* catchUp = Engine::Multiplayer::NetworkHostResource::CreatePacket(
            NewPlayerPosPacket,
            NewPlayerPos{existingPlayer.id,
                         registry.GetComponent<Engine::Position>(existingEntity)},
            true);
        Engine::Multiplayer::NetworkHostResource::Send(peer, catchUp, 0);
        // tell existing players about the enw player
        auto* announce = Engine::Multiplayer::NetworkHostResource::CreatePacket(
            NewPlayerPosPacket, NewPlayerPos{newPlayer.id, newPlayerPos}, true);
        Engine::Multiplayer::NetworkHostResource::Send(existingPlayer.peer, announce, 0);
    }
}

static void OnDisconnect(Engine::Registry& registry, ENetPeer* peer) {
    const auto player = Engine::Multiplayer::NetworkHostResource::GetEntityID(peer);
    if (!player.has_value()) {
        std::cout << "Cannot despawn" << std::endl;
        return;
    }
    registry.DestroyEntity(player.value());
    peer->data = nullptr;
}

int main() {
    std::cout << "======================================\n";
    std::cout << "          Server Sandbox               \n";
    std::cout << "======================================\n";
    Engine::Multiplayer::ENetContext context;
    Engine::ApplicationConfig config;
    config.windowTitle = "Server Demo";
    config.targetFPS = 60;
    ChangeDirectory(GetApplicationDirectory());

    Engine::Application app(config);

    Engine::PositionPlugin posPlugin;
    posPlugin.Init(app.GetRegistry());

    Engine::TextPlugin textPlugin;
    textPlugin.Init(app.GetRegistry());

    Engine::InputPlugin inputPlugin;
    inputPlugin.Init(app.GetRegistry());
    Engine::ShapePlugin shapePlugin;
    shapePlugin.Init(app.GetRegistry());

    app.GetRegistry()
        .RegisterComponent<Engine::Multiplayer::PlayerInformation>()
        ->RegisterComponent<KeyPress>()
        ->RegisterComponent<PlayerId>();

    auto& registry = app.GetRegistry();

    registry.AddResource<Engine::Multiplayer::NetworkHostResource>(
        Engine::Multiplayer::NetworkHostResource::CreateServer(1234, 2));

    auto& router =
        registry.AddResource<Engine::Multiplayer::PacketRouterResource>(OnConnect, OnDisconnect);
    router.RegisterHandler<KeyPress>(KeyPressPacket, KeyPressed);

    registry.RegisterSystem<Engine::MultiplayerSystem>();

    app.Run();

    return 0;
}
