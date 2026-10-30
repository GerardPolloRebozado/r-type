#include "Components/Position.hpp"
#include "Core/Application.hpp"
#include "ENetContext.hpp"
#include "Inputs/InputManager.hpp"
#include "Plugins/ColliderPlugin.hpp"
#include "Plugins/InputPlugin.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/TextPlugin.hpp"
#include "Resources/NetworkHostResource.hpp"
#include "Systems/MultiplayerSystem.hpp"

#include <iostream>
#include <raylib.h>

#include "PacketTypes.hpp"
#include "Plugins/MultiplayerPlugin.hpp"
#include "Plugins/OpacityPlugin.hpp"
#include "Plugins/ShapePlugin.hpp"
#include <cstddef>
#include <span>

class SendKeySystem : public Engine::System {
  public:
    ~SendKeySystem() noexcept override = default;

    void Update(Engine::Registry& registry, float dt) override {
        KeyPress keyPressed;
        if (IsKeyDown(KEY_D)) {
            keyPressed.key = 'd';
        }
        if (IsKeyDown(KEY_A)) {
            keyPressed.key = 'a';
        }
        if (IsKeyDown(KEY_W)) {
            keyPressed.key = 'w';
        }
        if (IsKeyDown(KEY_S)) {
            keyPressed.key = 's';
        }
        if (keyPressed.key == 0) {
            return;
        }
        auto& packetRegistry = registry.GetResource<Engine::Multiplayer::PacketRegistryResource>();
        ENetPacket* packet = Engine::Multiplayer::NetworkHostResource::CreatePacket(
            packetRegistry.GetOrRegister("key_press"), keyPressed, true);
        registry.GetResource<Engine::Multiplayer::NetworkHostResource>().Send(packet, 0);
    }

    void Render(Engine::Registry& registry) override {};
};

static void OnConnect(Engine::Registry& registry, ENetPeer* peer) {}

static void OnDisconnect(Engine::Registry& registry, ENetPeer* peer) {}

static void UpdatePlayerPosition(Engine::Registry& registry, const ENetPeer*,
                                 const NewPlayerPos& newPos) {
    auto& host = registry.GetResource<Engine::Multiplayer::NetworkHostResource>();
    auto playerEntity = host.GetEntityByNetworkId(newPos.id);
    if (!playerEntity.has_value()) {
        const auto entity = registry.CreateEntity();
        host.TrackNetworkEntity(registry, entity, newPos.id);
        constexpr auto shape = Engine::ShapeComponent{};
        shape.color = raylib::RED;

        registry.AddComponent(entity, shape);
        playerEntity = entity;
    }

    auto& pos = registry.RequireComponent<Engine::Position>(*playerEntity);
    pos = newPos.pos;
}

static void SpawnMyPlayer(Engine::Registry& registry, ENetPeer*,
                          const Engine::Multiplayer::YourPlayerId& player) {
    auto& host = registry.GetResource<Engine::Multiplayer::NetworkHostResource>();
    const auto playerEntity = registry.CreateEntity();
    constexpr auto shape = Engine::ShapeComponent{};
    shape.color = raylib::WHITE;

    registry.AddComponent(playerEntity, shape);
    registry.AddComponent(playerEntity, Engine::Multiplayer::YourPlayerId{player.id});
    host.TrackNetworkEntity(registry, playerEntity, player.id);
    registry.RegisterSystem<SendKeySystem>();
}

int main() {
    std::cout << "======================================\n";
    std::cout << "          Client Sandbox               \n";
    std::cout << "======================================\n";
    Engine::Multiplayer::ENetContext context;
    Engine::ApplicationConfig config;
    config.windowTitle = "Client Demo";
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

    Engine::Multiplayer::MultiplayerPlugin multiplayerPlugin;
    multiplayerPlugin.Init(app.GetRegistry());

    auto& registry = app.GetRegistry();
    registry.RegisterComponent<Engine::Multiplayer::YourPlayerId>();
    auto& host = registry.AddResource<Engine::Multiplayer::NetworkHostResource>(
        Engine::Multiplayer::NetworkHostResource::CreateClient(2));
    registry.RegisterComponent<KeyPress>();
    (void)host;
    auto* peer =
        registry.GetResource<Engine::Multiplayer::NetworkHostResource>().Connect("localhost", 3000);
    (void)peer;

    auto& packetRegistry = registry.GetResource<Engine::Multiplayer::PacketRegistryResource>();

    auto& router = registry.GetResource<Engine::Multiplayer::PacketRouterResource>();
    router.m_OnConnect = OnConnect;
    router.m_OnDisconnect = OnDisconnect;
    router.On(
        packetRegistry.GetOrRegister("your_player_id"),
        [](Engine::Registry& registry, ENetPeer* peer, const uint8_t* payload, std::size_t size) {
            Engine::Multiplayer::YourPlayerId assigned{};
            const std::span buffer{reinterpret_cast<const std::byte*>(payload), size};
            if (!glz::read_beve(assigned, buffer)) {
                SpawnMyPlayer(registry, peer, assigned);
            }
        });

    registry.RegisterSystem<Engine::MultiplayerSystem>();
    const auto textEntity = registry.CreateEntity();
    const auto& textComponent = Engine::Text("Use W/A/S/D to move, you are the white square", 60);
    registry.AddComponent(textEntity, textComponent);
    router.On(
        packetRegistry.GetOrRegister("new_player_pos"),
        [](Engine::Registry& registry, ENetPeer* peer, const uint8_t* payload, std::size_t size) {
            NewPlayerPos newPos{};
            const std::span buffer{reinterpret_cast<const std::byte*>(payload), size};
            if (!glz::read_beve(newPos, buffer)) {
                UpdatePlayerPosition(registry, peer, newPos);
            }
        });

    app.Run();

    return 0;
}