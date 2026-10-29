// Multiplayer scene switching over real ENet connections (localhost): one
// server and two clients in the same process, each with its own registry.
#include "Components/Name.hpp"
#include "Components/NetworkComponent.hpp"
#include "Components/SpriteComponent.hpp"
#include "ECS/Registry.hpp"
#include "Plugins/MultiplayerPlugin.hpp"
#include "Plugins/ReplicationPlugin.hpp"
#include "Resources/NetworkHostResource.hpp"
#include "Resources/PlayerSlotState.hpp"
#include "Resources/SceneManager.hpp"
#include "Scripting/LuaAPI/SceneAPI.hpp"
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <set>
#include <sol/sol.hpp>
#include <string>
#include <thread>
#include <vector>

static int failures = 0;
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::cerr << "FAIL line " << __LINE__ << ": " #cond "\n";                              \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

namespace {
using Engine::Registry;
using Engine::Multiplayer::NetworkHostResource;

void AddNetworked(Registry& r, const std::string& name, const std::string& authority) {
    const Engine::Entity entity = r.CreateEntity();
    r.AddComponent(entity, Engine::Name::Create(name));
    Engine::NetworkComponent network;
    network.authority = authority;
    r.AddComponent(entity, network);
}

// Same scenes on every machine, like scene files shipped with the game.
void RegisterScenes(Engine::SceneManager& scenes) {
    scenes.Register("lobby", [](Registry& r) {
        AddNetworked(r, "LobbySign", "Server");
        AddNetworked(r, "LobbyDoor", "Server");
        AddNetworked(r, "PlayerTemplate", "Client"); // removed by replication
    });
    scenes.Register("level", [](Registry& r) {
        AddNetworked(r, "Enemy1", "Server");
        AddNetworked(r, "Enemy2", "Server");
        AddNetworked(r, "Enemy3", "Server");
        AddNetworked(r, "PlayerTemplate", "Client");
    });
}

std::unique_ptr<Registry> MakeMachine(bool server, std::uint16_t port, const std::string& scene) {
    auto registry = std::make_unique<Registry>();
    Registry& r = *registry;
    r.RegisterComponent<Engine::Name>();
    r.RegisterComponent<Engine::SpriteComponent>();
    Engine::Multiplayer::ReplicationPlugin().Init(r);
    Engine::Multiplayer::MultiplayerPlugin().Init(r);
    if (server) {
        auto& host = r.AddResource<NetworkHostResource>(NetworkHostResource::CreateServer(
            port, Engine::Multiplayer::PlayerSlotState::kMaxPlayers));
        host.SetNextNetworkId(1'000'000);
    } else {
        auto& host = r.AddResource<NetworkHostResource>(NetworkHostResource::CreateClient());
        host.Connect("127.0.0.1", port);
    }
    auto& scenes = r.AddResource<Engine::SceneManager>();
    RegisterScenes(scenes);
    scenes.Load(scene);
    scenes.ApplyPending(r);
    return registry;
}

// One Application frame: systems, deferred destroys, scene switch.
void Frame(Registry& r) {
    const auto& systems = r.GetSystems();
    for (const auto& id : r.GetSystemUpdateOrder()) {
        systems.at(id)->Update(r, 1.0f / 60.0f);
    }
    r.FlushQueues();
    r.GetResource<Engine::SceneManager>().ApplyPending(r);
}

bool PumpUntil(const std::vector<Registry*>& machines, const std::function<bool()>& done) {
    for (int frame = 0; frame < 1500; ++frame) {
        for (Registry* machine : machines) {
            Frame(*machine);
        }
        if (done()) {
            // A few more frames so in-flight packets settle.
            for (int extra = 0; extra < 10; ++extra) {
                for (Registry* machine : machines) {
                    Frame(*machine);
                }
            }
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return false;
}

// Names of living networked entities with the given authority.
std::multiset<std::string> Networked(Registry& r, const std::string& authority) {
    std::multiset<std::string> names;
    for (const auto& [entity, network] : *r.GetComponentArray<Engine::NetworkComponent>()) {
        if (network.authority != authority || !r.IsEntityAlive(entity)) {
            continue;
        }
        names.insert(r.HasComponent<Engine::Name>(entity)
                         ? r.RequireComponent<Engine::Name>(entity).value
                         : std::string("player"));
    }
    return names;
}

std::set<std::size_t> PlayerIds(Registry& r) {
    std::set<std::size_t> ids;
    for (const auto& [entity, network] : *r.GetComponentArray<Engine::NetworkComponent>()) {
        if (network.authority == "Client" && r.IsEntityAlive(entity)) {
            ids.insert(network.networkId);
        }
    }
    return ids;
}

bool LocalPlayerReady(Registry& client) {
    auto& host = client.GetResource<NetworkHostResource>();
    const auto entity = host.GetLocalEntity();
    return host.GetLocalNetworkId().has_value() && entity.has_value() &&
           client.IsEntityAlive(*entity);
}

const std::string& Current(Registry& r) {
    return r.GetResource<Engine::SceneManager>().Current();
}
} // namespace

int main() {
    const auto port = static_cast<std::uint16_t>(
        42000 + std::chrono::steady_clock::now().time_since_epoch().count() % 2000);
    auto server = MakeMachine(true, port, "lobby");
    auto client1 = MakeMachine(false, port, "lobby");
    Registry& s = *server;
    Registry& c1 = *client1;

    // 1. Connect in the lobby.
    CHECK(PumpUntil({&s, &c1}, [&] { return LocalPlayerReady(c1); }));
    const auto c1Id = c1.GetResource<NetworkHostResource>().GetLocalNetworkId();
    CHECK(c1Id.has_value());
    CHECK((Networked(c1, "Server") == std::multiset<std::string>{"LobbyDoor", "LobbySign"}));
    CHECK(PlayerIds(s) == std::set<std::size_t>{*c1Id});

    // 2. The server switches: the client follows and gets its player back.
    s.GetResource<Engine::SceneManager>().Load("level");
    CHECK(PumpUntil({&s, &c1}, [&] { return Current(c1) == "level" && LocalPlayerReady(c1); }));
    CHECK(Current(s) == "level");
    const std::multiset<std::string> levelEnemies{"Enemy1", "Enemy2", "Enemy3"};
    CHECK(Networked(s, "Server") == levelEnemies);
    CHECK(Networked(c1, "Server") == levelEnemies); // no lobby leftovers
    CHECK(PlayerIds(s) == std::set<std::size_t>{*c1Id});
    CHECK(PlayerIds(c1) == std::set<std::size_t>{*c1Id});
    CHECK(c1.GetResource<NetworkHostResource>().GetLocalNetworkId() == c1Id);
    // Same color slot after the switch.
    CHECK(s.GetResource<Engine::Multiplayer::PlayerSlotState>().Get(*c1Id) ==
          std::optional<std::uint8_t>{0});

    // 3. Late join: a client starting on the lobby is moved to the level.
    auto client2 = MakeMachine(false, port, "lobby");
    Registry& c2 = *client2;
    CHECK(PumpUntil({&s, &c1, &c2}, [&] {
        return Current(c2) == "level" && LocalPlayerReady(c2) && PlayerIds(c1).size() == 2;
    }));
    const auto c2Id = c2.GetResource<NetworkHostResource>().GetLocalNetworkId();
    CHECK(c2Id.has_value() && c2Id != c1Id);
    CHECK(Networked(c2, "Server") == levelEnemies);
    CHECK(PlayerIds(c2) == (std::set<std::size_t>{*c1Id, *c2Id}));
    CHECK(PlayerIds(s) == (std::set<std::size_t>{*c1Id, *c2Id}));

    // 4. Reloading the same scene: everyone reloads, nothing is duplicated.
    s.GetResource<Engine::SceneManager>().Load("level");
    Frame(s); // server switches now
    CHECK(PumpUntil({&s, &c1, &c2}, [&] {
        return LocalPlayerReady(c1) && LocalPlayerReady(c2) && PlayerIds(c1).size() == 2 &&
               PlayerIds(c2).size() == 2;
    }));
    for (Registry* machine : {&s, &c1, &c2}) {
        CHECK(Networked(*machine, "Server") == levelEnemies);
        CHECK(PlayerIds(*machine) == (std::set<std::size_t>{*c1Id, *c2Id}));
    }

    // 5. Back to the lobby with two players.
    s.GetResource<Engine::SceneManager>().Load("lobby");
    CHECK(PumpUntil({&s, &c1, &c2}, [&] {
        return Current(c1) == "lobby" && Current(c2) == "lobby" && LocalPlayerReady(c1) &&
               LocalPlayerReady(c2) && PlayerIds(c1).size() == 2 && PlayerIds(c2).size() == 2;
    }));
    CHECK((Networked(c2, "Server") == std::multiset<std::string>{"LobbyDoor", "LobbySign"}));

    // 6. Lua on a client can't switch scenes: only the server decides.
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    Engine::LuaAPI::registerScene(lua, c1);
    lua.script("Scene.load('level')");
    CHECK(!c1.GetResource<Engine::SceneManager>().HasPendingLoad());
    CHECK(Current(c1) == "lobby");

    if (failures == 0) {
        std::cout << "MultiplayerScenesTest: all checks passed\n";
        return 0;
    }
    std::cerr << "MultiplayerScenesTest: " << failures << " check(s) failed\n";
    return 1;
}
