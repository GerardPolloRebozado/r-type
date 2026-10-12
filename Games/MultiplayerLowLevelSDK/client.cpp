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

struct KeyPress {
    char key = 0;

    template <typename S> void serialize(S& s) {
        s.value1b(key);
    }
};

const PacketTypeID KeyPressPacket = Engine::Multiplayer::Packets::GetPacketTypeID<KeyPress>();

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
        std::cout << "Sending key press" << std::endl;
        registry.GetResource<Engine::Multiplayer::NetworkHostResource>().Send(KeyPressPacket,
                                                                              keyPressed, 0, true);
    }

    void Render(Engine::Registry& registry) override {};
};

static void OnConnect(Engine::Registry& registry, ENetPeer* peer) {}

static void OnDisconnect(Engine::Registry& registry, ENetPeer* peer) {}

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

    auto& registry = app.GetRegistry();
    auto& resource = registry.AddResource<Engine::Multiplayer::NetworkHostResource>(
        Engine::Multiplayer::NetworkHostResource::CreateClient(2));
    auto* peer = resource.Connect("127.0.0.1", 1234);

    auto& router = registry.AddResource<Engine::Multiplayer::Packets::PacketRouterResource>(
        OnConnect, OnDisconnect);
    // router.RegisterHandler<KeyPress>(Engine::Multiplayer::Packets::GetPacketTypeID<KeyPress>(),
    //                                  KeyPressed);
    registry.RegisterSystem<Engine::MultiplayerSystem>();
    const auto textEntity = registry.CreateEntity();
    const auto& textComponent = Engine::Text("Any key press will be send to the server", 100);
    registry.AddComponent(textEntity, textComponent);
    registry.RegisterSystem<SendKeySystem>();

    app.Run();

    return 0;
}