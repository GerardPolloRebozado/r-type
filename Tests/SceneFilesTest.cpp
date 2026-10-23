#include "Assets/ProjectPaths.hpp"
#include "Components/AudioSource.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/CollisionEvents.hpp"
#include "Components/Input.hpp"
#include "Components/Name.hpp"
#include "Components/Opacity.hpp"
#include "Components/Parent.hpp"
#include "Components/ShapeComponent.hpp"
#include "Components/SignalConnections.hpp"
#include "Components/SpriteComponent.hpp"
#include "Components/Text.hpp"
#include "ECS/Registry.hpp"
#include "Resources/SceneManager.hpp"
#include "SceneConfig.hpp"
#include "SceneFiles.hpp"
#include "SceneSerializer.hpp"
#include "Scripting/LuaAPI/SceneAPI.hpp"
#include "Scripting/ScriptComponent.hpp"
#include <filesystem>
#include <iostream>
#include <set>
#include <sol/sol.hpp>
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
namespace fs = std::filesystem;

// The scene serializer reads every known component type. In a game the
// plugins register them; here we do it by hand.
void RegisterAll(Engine::Registry& reg) {
    reg.RegisterComponent<Engine::Name>();
    reg.RegisterComponent<Engine::SpriteComponent>();
    reg.RegisterComponent<Engine::ShapeComponent>();
    reg.RegisterComponent<Engine::Text>();
    reg.RegisterComponent<Engine::ColliderComponent>();
    reg.RegisterComponent<Engine::Parent>();
    reg.RegisterComponent<Engine::Input>();
    reg.RegisterComponent<Engine::AudioSource>();
    reg.RegisterComponent<Engine::Opacity>();
    reg.RegisterComponent<Engine::CameraComponent>();
    reg.RegisterComponent<Engine::ScriptComponent>();
    reg.RegisterComponent<Engine::SignalConnections>();
    reg.RegisterComponent<Engine::CollisionEvents>();
}

// Writes a scene file holding one entity per name.
void WriteScene(const fs::path& path, const std::set<std::string>& names) {
    Engine::Registry registry;
    RegisterAll(registry);
    for (const auto& name : names) {
        registry.AddComponent(registry.CreateEntity(), Engine::Name::Create(name));
    }
    Serialization::SceneConfig config;
    config.m_ProjectFolder = path.parent_path().string();
    config.windowTitle = "Title from " + path.filename().string();
    Serialization::SceneSerializer serializer;
    serializer.Serialize(path.string(), registry, config);
}

std::set<std::string> AliveNames(Engine::Registry& registry) {
    std::set<std::string> names;
    for (std::size_t i = 1; i < registry.GetEntityCount(); ++i) {
        if (registry.IsEntityAlive(i) && registry.HasComponent<Engine::Name>(i)) {
            names.insert(registry.GetComponent<Engine::Name>(i).value);
        }
    }
    return names;
}

std::size_t CountAlive(const Engine::Registry& registry) {
    std::size_t alive = 0;
    for (std::size_t i = 1; i < registry.GetEntityCount(); ++i) {
        alive += registry.IsEntityAlive(i) ? 1 : 0;
    }
    return alive;
}
} // namespace

int main() {
    const fs::path project = fs::temp_directory_path() / "rtype_scene_files_test";
    fs::remove_all(project);
    fs::create_directories(project / "levels");
    WriteScene(project / "Menu.scene", {"StartButton", "Logo"});
    WriteScene(project / "levels" / "Level1.scene", {"Player", "Enemy", "Wall"});

    Engine::Registry registry;
    RegisterAll(registry);
    // The game knows its project folder: relative scene paths resolve against it.
    registry.AddResource<Engine::ProjectContext>(project.string());

    auto& scenes = Serialization::EnableSceneFiles(registry);
    CHECK(registry.HasResource<Engine::SceneManager>());
    CHECK(scenes.HasFileLoader());
    // Calling it again reuses the same manager.
    CHECK(&Serialization::EnableSceneFiles(registry) == &scenes);

    // File scene, relative to the project folder.
    scenes.Load("Menu.scene");
    CHECK(scenes.ApplyPending(registry));
    CHECK(scenes.Current() == "Menu.scene");
    CHECK((AliveNames(registry) == std::set<std::string>{"StartButton", "Logo"}));

    // File scene in a subfolder: Menu's entities are gone, not merged.
    scenes.Load("levels/Level1.scene");
    CHECK(scenes.ApplyPending(registry));
    CHECK((AliveNames(registry) == std::set<std::string>{"Player", "Enemy", "Wall"}));

    // Code scenes and file scenes mix freely.
    scenes.Register("gameover", [](Engine::Registry& r) {
        r.AddComponent(r.CreateEntity(), Engine::Name::Create("GameOverText"));
    });
    scenes.Load("gameover");
    CHECK(scenes.ApplyPending(registry));
    CHECK((AliveNames(registry) == std::set<std::string>{"GameOverText"}));

    // Back and forth: no leftovers, IDs don't keep growing.
    for (int i = 0; i < 10; ++i) {
        scenes.Load(i % 2 == 0 ? "Menu.scene" : "levels/Level1.scene");
        scenes.ApplyPending(registry);
    }
    CHECK(scenes.Current() == "levels/Level1.scene");
    CHECK(CountAlive(registry) == 3);
    CHECK(registry.GetEntityCount() == 4);

    // A missing file is rejected and the current scene keeps running.
    scenes.Load("Nope.scene");
    CHECK(!scenes.ApplyPending(registry));
    CHECK(scenes.Current() == "levels/Level1.scene");
    CHECK(CountAlive(registry) == 3);

    // Project folder is a resource: it survives every switch.
    CHECK(registry.GetResource<Engine::ProjectContext>().projectFolder == project.string());

    // Lua: Scene.load / Scene.current go through the same manager.
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    Engine::LuaAPI::registerScene(lua, registry);
    CHECK(lua.script("return Scene.current()").get<std::string>() == "levels/Level1.scene");
    lua.script("Scene.load('Menu.scene')");
    CHECK(scenes.HasPendingLoad());
    // Nothing changes until the end of the frame.
    CHECK(CountAlive(registry) == 3);
    scenes.ApplyPending(registry);
    CHECK(lua.script("return Scene.current()").get<std::string>() == "Menu.scene");
    CHECK((AliveNames(registry) == std::set<std::string>{"StartButton", "Logo"}));

    // Lua without a SceneManager: logs an error instead of crashing.
    Engine::Registry bare;
    sol::state lua2;
    lua2.open_libraries(sol::lib::base);
    Engine::LuaAPI::registerScene(lua2, bare);
    lua2.script("Scene.load('Menu.scene')");
    CHECK(lua2.script("return Scene.current()").get<std::string>().empty());

    // Startup scene: a .scene argument wins, flags are ignored.
    using Serialization::PickStartupScene;
    CHECK(PickStartupScene({}, "test.scene") == "test.scene");
    CHECK(PickStartupScene({"--server", "--headless"}, "test.scene") == "test.scene");
    CHECK(PickStartupScene({"--client", "Level2.scene"}, "test.scene") == "Level2.scene");
    CHECK(PickStartupScene({"levels/Level1.scene"}, "test.scene") == "levels/Level1.scene");
    CHECK(PickStartupScene({".scene"}, "test.scene") == "test.scene");

    fs::remove_all(project);

    if (failures == 0) {
        std::cout << "SceneFilesTest: all checks passed\n";
        return 0;
    }
    std::cerr << "SceneFilesTest: " << failures << " check(s) failed\n";
    return 1;
}
