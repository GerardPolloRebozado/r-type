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
    auto& host = registry.GetResource<Engine::Multiplayer::NetworkHostResource>();
    const auto entity = Engine::Multiplayer::NetworkHostResource::GetEntityID(peer);
    if (!entity.has_value()) {
        return;
    }
    const auto networkId = host.GetNetworkId(registry, entity.value());
    if (!networkId.has_value()) {
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
    const auto newPosition = NewPlayerPos{*networkId, position};
    const auto newPosNotification = Engine::Multiplayer::NetworkHostResource::CreatePacket(
        NewPlayerPosPacket, newPosition, false);
    registry.GetResource<Engine::Multiplayer::NetworkHostResource>().SendAllHost(newPosNotification,
                                                                                 0);
}

static void OnConnect(Engine::Registry& registry, ENetPeer* peer) {
    // The SDK already created the player entity, assigned its network id and
    // told the client about it. We only add game visuals and catch everyone up.
    auto& host = registry.GetResource<Engine::Multiplayer::NetworkHostResource>();
    const auto playerEntity = Engine::Multiplayer::NetworkHostResource::GetEntityID(peer);
    if (!playerEntity.has_value()) {
        return;
    }
    constexpr Engine::ShapeComponent shape;
    registry.AddComponent(*playerEntity, shape);

    const auto newPlayerId = host.GetNetworkId(registry, *playerEntity);
    if (!newPlayerId.has_value()) {
        return;
    }

    const auto& newPlayerPos = registry.GetComponent<Engine::Position>(*playerEntity);
    auto playerArray = registry.GetComponentArray<Engine::Multiplayer::PlayerInformation>();
    for (const auto& [existingEntity, existingPlayer] : *playerArray) {
        if (existingPlayer.id == *newPlayerId) {
            continue;
        }
        const auto existingPeer = host.GetPeer(existingEntity);
        if (!existingPeer.has_value()) {
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
            NewPlayerPosPacket, NewPlayerPos{*newPlayerId, newPlayerPos}, true);
        Engine::Multiplayer::NetworkHostResource::Send(*existingPeer, announce, 0);
    }
}

static void OnDisconnect(Engine::Registry& registry, ENetPeer* peer) {
    // The SDK already destroyed the player entity and cleared its id mapping.
    (void)registry;
    (void)peer;
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

    auto& registry = app.GetRegistry();

    auto& host = registry.AddResource<Engine::Multiplayer::NetworkHostResource>(
        Engine::Multiplayer::NetworkHostResource::CreateServer(3000, 10));
    Engine::Multiplayer::NetworkHostResource::RegisterMultiplayerComponents(registry);
    (void)host;

    registry.RegisterComponent<KeyPress>();

    auto& router =
        registry.AddResource<Engine::Multiplayer::PacketRouterResource>(OnConnect, OnDisconnect);
    router.RegisterHandler<KeyPress>(KeyPressPacket, KeyPressed);

    registry.RegisterSystem<Engine::MultiplayerSystem>();

    app.Run();

    return 0;
}
