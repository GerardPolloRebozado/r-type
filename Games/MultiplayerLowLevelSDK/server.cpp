#include "Components/Position.hpp"
#include "Core/Application.hpp"
#include "ENetContext.hpp"
#include "Inputs/InputManager.hpp"
#include "Plugins/ColliderPlugin.hpp"
#include "Plugins/InputPlugin.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/ShapePlugin.hpp"
#include "Plugins/TextPlugin.hpp"
#include "Resources/NetworkHostResource.hpp"
#include "Systems/MultiplayerSystem.hpp"

#include <iostream>
#include <raylib.h>

struct KeyPress {
    char key;

    template <typename S> void serialize(S& s) {
        s.value1b(key);
    }
};

static void KeyPressed(Engine::Registry& registry, ENetPeer* peer, const KeyPress& keyPress) {
    std::cout << "New key press: " << keyPress.key << std::endl;
    const auto entity = Engine::Multiplayer::NetworkHostResource::GetEntityID(peer);
    if (!entity.has_value()) {
        return;
    }
    std::cout << "New key press: " << keyPress.key << std::endl;
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
}

static void OnConnect(Engine::Registry& registry, ENetPeer* peer) {
    auto player = registry.CreateEntity();
    Engine::ShapeComponent shape;
    shape.type = Engine::ShapeType::RECTANGLE;
    registry.AddComponent(player, shape);
    Engine::Multiplayer::NetworkHostResource::SetPeerEntity(peer, player);
}

static void OnDisconnect(Engine::Registry& registry, ENetPeer* peer) {
    auto player = Engine::Multiplayer::NetworkHostResource::GetEntityID(peer);
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

    auto& registry = app.GetRegistry();

    registry.AddResource<Engine::Multiplayer::NetworkHostResource>(
        Engine::Multiplayer::NetworkHostResource::CreateServer(1234, 2));

    auto& router = registry.AddResource<Engine::Multiplayer::Packets::PacketRouterResource>(
        OnConnect, OnDisconnect);
    router.RegisterHandler<KeyPress>(Engine::Multiplayer::Packets::GetPacketTypeID<KeyPress>(),
                                     KeyPressed);

    registry.RegisterSystem<Engine::MultiplayerSystem>();

    app.Run();

    return 0;
}