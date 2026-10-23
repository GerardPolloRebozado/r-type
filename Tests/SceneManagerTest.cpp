#include "ECS/Registry.hpp"
#include "ECS/System.hpp"
#include "Resources/SceneManager.hpp"
#include <iostream>
#include <string>
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
struct Marker {
    int value = 0;
};

// Records unload calls and checks entities are still alive at that moment.
struct TrackingSystem : Engine::System {
    int unloadCount = 0;
    bool entitiesAliveOnUnload = false;
    void OnSceneUnloaded(Engine::Registry& registry) override {
        ++unloadCount;
        entitiesAliveOnUnload = registry.IsEntityAlive(Engine::Entity{1});
    }
};

std::size_t CountAlive(const Engine::Registry& registry) {
    std::size_t alive = 0;
    for (std::size_t i = 1; i < registry.GetEntityCount(); ++i) {
        if (registry.IsEntityAlive(Engine::Entity{i})) {
            ++alive;
        }
    }
    return alive;
}

void TestClearEntities() {
    Engine::Registry registry;
    registry.RegisterComponent<Marker>();
    auto system = registry.RegisterSystem<TrackingSystem>();

    const Engine::Entity a = registry.CreateEntity();
    const Engine::Entity b = registry.CreateEntity();
    registry.AddComponent<Marker>(a, Marker{7});
    system->m_Entities.insert(a);
    registry.QueueDestroyEntity(b);

    registry.ClearEntities();

    CHECK(!registry.IsEntityAlive(a));
    CHECK(!registry.IsEntityAlive(b));
    CHECK(system->m_Entities.empty());
    CHECK(registry.GetEntityCount() == 1);
    // IDs restart at 1, and component types stay registered.
    const Engine::Entity fresh = registry.CreateEntity();
    CHECK(fresh == Engine::Entity{1});
    CHECK(!registry.HasComponent<Marker>(fresh));
    registry.FlushQueues(); // old queued destroy of id 2 must be gone
    CHECK(registry.IsEntityAlive(fresh));
}

void TestCodeSceneSwitch() {
    Engine::Registry registry;
    registry.RegisterComponent<Marker>();
    auto system = registry.RegisterSystem<TrackingSystem>();
    auto& scenes = registry.AddResource<Engine::SceneManager>();

    scenes.Register("menu", [](Engine::Registry& r) {
        r.AddComponent<Marker>(r.CreateEntity(), Marker{1});
    });
    scenes.Register("level", [](Engine::Registry& r) {
        for (int i = 0; i < 3; ++i) {
            r.AddComponent<Marker>(r.CreateEntity(), Marker{2});
        }
    });
    std::vector<std::string> loaded;
    scenes.OnLoaded([&loaded](Engine::Registry&, const std::string& name) {
        loaded.push_back(name);
    });

    // Load only queues: nothing changes until ApplyPending.
    scenes.Load("menu");
    CHECK(scenes.HasPendingLoad());
    CHECK(CountAlive(registry) == 0);
    CHECK(scenes.ApplyPending(registry));
    CHECK(!scenes.HasPendingLoad());
    CHECK(scenes.Current() == "menu");
    CHECK(CountAlive(registry) == 1);
    CHECK(registry.GetComponent<Marker>(Engine::Entity{1}).value == 1);

    // Switch back and forth: old entities go, IDs don't keep growing.
    for (int i = 0; i < 20; ++i) {
        scenes.Load(i % 2 == 0 ? "level" : "menu");
        scenes.ApplyPending(registry);
    }
    CHECK(scenes.Current() == "menu");
    CHECK(CountAlive(registry) == 1);
    CHECK(registry.GetEntityCount() == 2);
    CHECK(system->unloadCount == 21);
    CHECK(system->entitiesAliveOnUnload);
    CHECK(loaded.size() == 21);
    CHECK(loaded.back() == "menu");

    // Nothing pending: no switch.
    CHECK(!scenes.ApplyPending(registry));
}

void TestUnknownSceneKeepsCurrent() {
    Engine::Registry registry;
    registry.RegisterComponent<Marker>();
    auto& scenes = registry.AddResource<Engine::SceneManager>();
    scenes.Register("menu", [](Engine::Registry& r) {
        r.AddComponent<Marker>(r.CreateEntity(), Marker{1});
    });
    scenes.Load("menu");
    scenes.ApplyPending(registry);

    // No file loader installed: unknown name is rejected.
    scenes.Load("does-not-exist");
    CHECK(!scenes.ApplyPending(registry));
    CHECK(scenes.Current() == "menu");
    CHECK(CountAlive(registry) == 1);

    // With a file loader, a missing file is still rejected before clearing.
    bool loaderCalled = false;
    scenes.SetFileLoader([&loaderCalled](Engine::Registry&, const std::string&) {
        loaderCalled = true;
        return true;
    });
    CHECK(scenes.HasFileLoader());
    scenes.Load("missing-file.scene");
    CHECK(!scenes.ApplyPending(registry));
    CHECK(!loaderCalled);
    CHECK(scenes.Current() == "menu");
    CHECK(CountAlive(registry) == 1);
}

void TestLoadFromBuilderWaitsForNextApply() {
    Engine::Registry registry;
    auto& scenes = registry.AddResource<Engine::SceneManager>();
    scenes.Register("intro", [&scenes](Engine::Registry& r) {
        r.CreateEntity();
        scenes.Load("menu");
    });
    scenes.Register("menu", [](Engine::Registry& r) { r.CreateEntity(); });

    scenes.Load("intro");
    scenes.ApplyPending(registry);
    CHECK(scenes.Current() == "intro");
    CHECK(scenes.HasPendingLoad());
    scenes.ApplyPending(registry);
    CHECK(scenes.Current() == "menu");
}
} // namespace

int main() {
    TestClearEntities();
    TestCodeSceneSwitch();
    TestUnknownSceneKeepsCurrent();
    TestLoadFromBuilderWaitsForNextApply();

    if (failures == 0) {
        std::cout << "SceneManagerTest: all checks passed\n";
        return 0;
    }
    std::cerr << "SceneManagerTest: " << failures << " check(s) failed\n";
    return 1;
}
