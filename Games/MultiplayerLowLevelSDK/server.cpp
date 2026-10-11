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
    char key;

    template <typename S> void serialize(S& s) {
        s.value1b(key);
    }
};

struct TextTag {};

static void KeyPressed(Engine::Registry& registry, ENetPeer*, const KeyPress& keyPress) {
    auto textArray = registry.GetComponentArray<TextTag>();
    auto& textCompontent = registry.GetComponent<Engine::Text>(textArray->begin()->first);
    textCompontent.text = keyPress.key;
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

    auto& registry = app.GetRegistry();

    registry.AddResource<Engine::Multiplayer::NetworkHostResource>(
        Engine::Multiplayer::NetworkHostResource::CreateServer(1234, 2));

    auto& router = registry.AddResource<Engine::Multiplayer::Packets::PacketRouterResource>();
    router.RegisterHandler<KeyPress>(Engine::Multiplayer::Packets::GetPacketTypeID<KeyPress>(),
                                     KeyPressed);

    registry.RegisterSystem<Engine::MultiplayerSystem>();
    const auto textEntity = registry.CreateEntity();
    const auto& textComponent = Engine::Text("No key pressed", 120);
    registry.AddComponent(textEntity, textComponent);
    registry.RegisterComponent<TextTag>();
    registry.AddComponent(textEntity, TextTag{});

    app.Run();

    return 0;
}