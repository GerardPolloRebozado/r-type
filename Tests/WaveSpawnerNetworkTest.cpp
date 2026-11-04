// WaveSpawnerSystem must only spawn waves on the authoritative process:
// the server (replicated to clients) or single-player. A networked client
// spawning its own waves duplicates every enemy and leaks them forever.
#include "Components/CameraComponent.hpp"
#include "Components/Position.hpp"
#include "ECS/Registry.hpp"
#include "Resources/NetworkHostResource.hpp"
#include "Resources/PrefabSpawner.hpp"
#include "Resources/WaveSchedule.hpp"
#include "Systems/WaveSpawnerSystem.hpp"
#include <iostream>
#include <memory>
#include <optional>
#include <string>

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

// `role` empty = single-player (no host), false = client, true = server.
std::unique_ptr<Registry> MakeRegistry(std::optional<bool> role) {
    auto registry = std::make_unique<Registry>();
    Registry& r = *registry;
    r.RegisterComponent<Engine::CameraComponent>();

    Engine::WaveSchedule schedule;
    Engine::WaveDefinition wave;
    wave.name = "Test";
    wave.triggerX = 0.0f;
    wave.prefabPath = "test";
    wave.formation = Engine::WavePatternType::Line;
    wave.count = 3;
    wave.spacing = 10.0f;
    wave.baseY = 0.0f;
    wave.spawnOffsetX = 100.0f;
    wave.attachParent = false;
    schedule.waves.push_back(wave);
    r.AddResource<Engine::WaveSchedule>(schedule);

    Engine::PrefabSpawner spawner;
    spawner.Spawn = [](Registry& reg, const std::string& /*path*/) { return reg.CreateEntity(); };
    r.AddResource<Engine::PrefabSpawner>(spawner);

    // Camera already past the trigger.
    const Engine::Entity camera = r.CreateEntity();
    r.AddComponent<Engine::CameraComponent>(camera, Engine::CameraComponent{});
    auto& position = r.RequireComponent<Engine::Position>(camera);
    position.x = 100.0f;
    position.prevX = 100.0f;

    if (role.has_value()) {
        if (*role) {
            r.AddResource<NetworkHostResource>(NetworkHostResource::CreateServer(34567, 4));
        } else {
            r.AddResource<NetworkHostResource>(NetworkHostResource::CreateClient());
        }
    }

    return registry;
}

std::size_t SpawnedByUpdate(Registry& r) {
    Engine::WaveSpawnerSystem system;
    const std::size_t before = r.GetEntityCount();
    system.Update(r, 1.0f / 60.0f);
    return r.GetEntityCount() - before;
}

void TestSinglePlayerSpawns() {
    auto registry = MakeRegistry(std::nullopt);
    CHECK(SpawnedByUpdate(*registry) == 3);
}

void TestServerSpawns() {
    auto registry = MakeRegistry(true);
    CHECK(SpawnedByUpdate(*registry) == 3);
}

void TestClientDoesNotSpawn() {
    auto registry = MakeRegistry(false);
    CHECK(SpawnedByUpdate(*registry) == 0);
}
} // namespace

int main() {
    TestSinglePlayerSpawns();
    TestServerSpawns();
    TestClientDoesNotSpawn();

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "WaveSpawnerNetworkTest: all checks passed\n";
    return 0;
}
