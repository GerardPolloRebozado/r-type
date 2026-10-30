#include "Assets/ProjectPaths.hpp"
#include "Components/AudioSource.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/CollisionEvents.hpp"
#include "Components/Input.hpp"
#include "Components/Name.hpp"
#include "Components/NetworkComponent.hpp"
#include "Components/Opacity.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Components/PrefabInstance.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "Components/SignalConnections.hpp"
#include "Components/SpriteComponent.hpp"
#include "Components/Text.hpp"
#include "ECS/Registry.hpp"
#include "GlazeSerializer.hpp"
#include "RuntimePrefab.hpp"
#include "SceneConfig.hpp"
#include "SceneSerializer.hpp"
#include "Scripting/ScriptComponent.hpp"
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

static int failures = 0;
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::cerr << "FAIL line " << __LINE__ << ": " #cond "\n";                              \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

static void RegisterAll(Engine::Registry& reg) {
    reg.RegisterComponent<Engine::Name>();
    reg.RegisterComponent<Engine::Position>();
    reg.RegisterComponent<Engine::Rotation>();
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

static void BuildScene(Engine::Registry& reg, Serialization::SceneConfig& config) {
    RegisterAll(reg);

    auto player = reg.CreateEntity();
    reg.AddComponent(player, Engine::Name::Create("Player"));
    reg.AddComponent(player, Engine::Position(10.0f, 20.0f));
    reg.AddComponent(player, Engine::Rotation::Create(45.0f));
    Engine::ShapeComponent shape;
    shape.type = Engine::ShapeType::RECTANGLE;
    shape.width = 100.0f;
    shape.height = 50.0f;
    shape.color = Color{10, 20, 30, 40};
    reg.AddComponent(player, shape);
    reg.AddComponent(player, Engine::Text("hello", 12.0f, Color{1, 2, 3, 4}));
    reg.AddComponent(player, Engine::Input::Create(2, false));
    Engine::AudioSource audio;
    audio.assetId = "sfx/laser.wav";
    audio.isMusic = false;
    audio.volume = 0.5f;
    audio.loop = true;
    reg.AddComponent(player, audio);

    auto enemy = reg.CreateEntity();
    reg.AddComponent(enemy, Engine::Name::Create("Enemy"));
    reg.AddComponent(enemy, Engine::Position(30.0f, 40.0f));
    Engine::ShapeComponent circle;
    circle.type = Engine::ShapeType::CIRCLE;
    circle.radius = 10.0f;
    reg.AddComponent(enemy, Engine::ColliderComponent::Create(circle, true, true));
    reg.AddComponent(enemy, Engine::Parent::Create(player));

    auto ship = reg.CreateEntity();
    reg.AddComponent(ship, Engine::Rotation::Create(90.0f));
    reg.AddComponent(ship,
                     Engine::SpriteComponent(nullptr, "assets/ship.png", 5.0f, 6.0f, 2.0f, 3.0f));

    config.windowWidth = 1920;
    config.windowHeight = 1080;
    config.windowTitle = "SerdeTest";
    config.targetFPS = 144;
    config.m_ProjectFolder = "/tmp/proj";
    config.m_BuildExecutableName = "SerdeGame";
    config.m_BuildWindowTitle = "SerdeGameTitle";
    config.m_InitialScenePath = "/tmp/proj/test.scene";
    config.m_InputMap["Jump"] = {{Engine::Keybind{32, 0, Engine::KEYBOARD}}, false};
    config.m_InputMap["Fire"] = {{Engine::Keybind{1, 0, Engine::DeviceType::MOUSE}}, true};
}

static Engine::Entity FindByName(Engine::Registry& reg, const std::string& name) {
    for (std::size_t i = 0; i < reg.GetEntityCount(); ++i) {
        if (reg.HasComponent<Engine::Name>(i) && reg.RequireComponent<Engine::Name>(i).value == name) {
            return i;
        }
    }
    return static_cast<Engine::Entity>(-1);
}

static std::string ReadText(const std::filesystem::path& path) {
    std::ifstream file(path);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

static void WriteText(const std::filesystem::path& path, const std::string& text) {
    std::ofstream(path, std::ios::trunc) << text;
}

static std::string ReplaceAll(std::string text, const std::string& from, const std::string& to) {
    for (std::size_t pos = 0; (pos = text.find(from, pos)) != std::string::npos;
         pos += to.size()) {
        text.replace(pos, from.size(), to);
    }
    return text;
}

static bool SameColor(const Color& a, const Color& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

// Shape colors must be saved as "color" on every compiler, and files saved
// with the old "color)" key (some compilers did that) must still load.
static void TestShapeColorKey() {
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() / "rtype_color_key_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    const Color expected{10, 20, 30, 40};

    Engine::Registry reg;
    RegisterAll(reg);
    const Engine::Entity box = reg.CreateEntity();
    reg.AddComponent(box, Engine::Name::Create("Box"));
    Engine::ShapeComponent shape;
    shape.color = expected;
    reg.AddComponent(box, shape);
    Engine::ColliderComponent collider;
    collider.shape = shape;
    reg.AddComponent(box, collider);

    Serialization::SceneConfig config;
    Serialization::SceneSerializer writer;
    writer.Serialize((dir / "new.scene").string(), reg, config);
    const std::string saved = ReadText(dir / "new.scene");
    CHECK(saved.find("\"color\":") != std::string::npos);
    CHECK(saved.find("\"color)\"") == std::string::npos);

    // Old scene file: same content with the legacy key.
    WriteText(dir / "legacy.scene", ReplaceAll(saved, "\"color\":", "\"color)\":"));
    Engine::Registry legacy;
    RegisterAll(legacy);
    Serialization::SceneSerializer reader;
    CHECK(reader.Deserialize((dir / "legacy.scene").string(), legacy, std::nullopt));
    const Engine::Entity loaded = FindByName(legacy, "Box");
    CHECK(loaded != static_cast<Engine::Entity>(-1));
    if (loaded != static_cast<Engine::Entity>(-1)) {
        CHECK(SameColor(legacy.RequireComponent<Engine::ShapeComponent>(loaded).color, expected));
        CHECK(SameColor(legacy.RequireComponent<Engine::ColliderComponent>(loaded).shape->color,
                        expected));
    }

    // Old prefab file: prefabs are read through the same serializer.
    Serialization::GlazeSerializer glaze;
    const std::string prefab = glaze.SerializeToString(glaze.CaptureScene(reg));
    WriteText(dir / "legacy.prefab", ReplaceAll(prefab, "\"color\":", "\"color)\":"));
    Engine::Registry spawned;
    RegisterAll(spawned);
    const Engine::Entity root =
        Serialization::SpawnPrefabRuntime(spawned, (dir / "legacy.prefab").string());
    CHECK(root != static_cast<Engine::Entity>(-1));
    if (root != static_cast<Engine::Entity>(-1)) {
        CHECK(SameColor(spawned.RequireComponent<Engine::ShapeComponent>(root).color, expected));
    }

    fs::remove_all(dir);
}

// The scene and prefabs shipped with R-Type load on this OS.
static void TestRTypeFilesLoad() {
#ifdef RTYPE_SOURCE_DIR
    namespace fs = std::filesystem;
    const fs::path project = fs::path(RTYPE_SOURCE_DIR) / "R-Type";
    Engine::Registry reg;
    RegisterAll(reg);
    reg.RegisterComponent<Engine::NetworkComponent>();
    reg.RegisterComponent<Engine::PrefabInstance>();
    Serialization::SceneSerializer reader;
    CHECK(reader.Deserialize((project / "test.scene").string(), reg, std::nullopt));
    CHECK(FindByName(reg, "Player") != static_cast<Engine::Entity>(-1));
    for (const char* prefab : {"Prefabs/Player.prefab", "Prefabs/Bink.prefab",
                               "Prefabs/Pata-Pata.prefab"}) {
        CHECK(Serialization::SpawnPrefabRuntime(reg, (project / prefab).string()) !=
              static_cast<Engine::Entity>(-1));
    }
#endif
}

static void VerifyScene(Engine::Registry& reg, const Serialization::SceneConfig& config) {
    CHECK(reg.GetEntityCount() == 4); // 1 initial slot + 3 created entities

    Engine::Entity player = FindByName(reg, "Player");
    Engine::Entity enemy = FindByName(reg, "Enemy");
    CHECK(player != static_cast<Engine::Entity>(-1));
    CHECK(enemy != static_cast<Engine::Entity>(-1));

    const auto& pos = reg.RequireComponent<Engine::Position>(player);
    CHECK(pos.x == 10.0f);
    CHECK(pos.y == 20.0f);
    CHECK(reg.RequireComponent<Engine::Rotation>(player).angle == 45.0f);

    const auto& shape = reg.RequireComponent<Engine::ShapeComponent>(player);
    CHECK(shape.type == Engine::ShapeType::RECTANGLE);
    CHECK(shape.width == 100.0f);
    CHECK(shape.height == 50.0f);
    CHECK(shape.color.r == 10);
    CHECK(shape.color.g == 20);
    CHECK(shape.color.b == 30);
    CHECK(shape.color.a == 40);

    const auto& text = reg.RequireComponent<Engine::Text>(player);
    CHECK(text.text == "hello");
    CHECK(text.fontsize == 12.0f);
    CHECK(text.color.r == 1);
    CHECK(text.color.a == 4);

    const auto& input = reg.RequireComponent<Engine::Input>(player);
    CHECK(input.playerIdx == 2);
    CHECK(input.isActive == false);

    const auto& audio = reg.RequireComponent<Engine::AudioSource>(player);
    CHECK(audio.assetId == "sfx/laser.wav");
    CHECK(audio.volume == 0.5f);
    CHECK(audio.loop == true);

    const auto& ecol = reg.RequireComponent<Engine::ColliderComponent>(enemy);
    CHECK(ecol.enabled == true);
    CHECK(ecol.isTrigger == true);
    CHECK(ecol.shape.has_value());
    CHECK(ecol.shape->type == Engine::ShapeType::CIRCLE);
    CHECK(ecol.shape->radius == 10.0f);
    CHECK(reg.RequireComponent<Engine::Parent>(enemy).parent == player);

    // Unnamed sprite entity: no Name component
    bool foundSprite = false;
    for (std::size_t i = 0; i < reg.GetEntityCount(); ++i) {
        if (!reg.HasComponent<Engine::Name>(i) && reg.HasComponent<Engine::SpriteComponent>(i)) {
            const auto& spr = reg.RequireComponent<Engine::SpriteComponent>(i);
            CHECK(spr.assetPath == "assets/ship.png");
            CHECK(spr.offsetX == 5.0f);
            CHECK(spr.offsetY == 6.0f);
            CHECK(spr.scaleX == 2.0f);
            CHECK(spr.scaleY == 3.0f);
            CHECK(reg.RequireComponent<Engine::Rotation>(i).angle == 90.0f);
            foundSprite = true;
        }
    }
    CHECK(foundSprite);

    CHECK(config.windowWidth == 1920);
    CHECK(config.windowHeight == 1080);
    CHECK(config.windowTitle == "SerdeTest");
    CHECK(config.targetFPS == 144);
    CHECK(config.m_ProjectFolder == "/local/machine/proj");
    CHECK(config.m_BuildExecutableName == "SerdeGame");
    CHECK(config.m_BuildWindowTitle == "SerdeGameTitle");
    CHECK(config.m_InitialScenePath == "/local/machine/proj/local.scene");
    CHECK(config.m_InputMap.at("Jump").keybinds.size() == 1);
    CHECK(config.m_InputMap.at("Jump").keybinds[0].key == 32);
    CHECK(config.m_InputMap.at("Jump").keybinds[0].type == Engine::DeviceType::KEYBOARD);
    CHECK(config.m_InputMap.at("Jump").networked == false);
    CHECK(config.m_InputMap.at("Fire").keybinds[0].type == Engine::DeviceType::MOUSE);
    CHECK(config.m_InputMap.at("Fire").networked == true);
}

int main() {
    using SerializationFormat = Serialization::SerializationFormat;

    for (auto format : {SerializationFormat::JSON, SerializationFormat::Binary}) {
        const char* path =
            (format == SerializationFormat::JSON) ? "serde_test.scene" : "serde_test.bscene";

        Engine::Registry reg;
        Serialization::SceneConfig config;
        BuildScene(reg, config);

        Serialization::SceneSerializer writer;
        writer.Serialize(path, reg, config, format);

        Engine::Registry loaded;
        RegisterAll(loaded);
        Serialization::SceneConfig loadedConfig;
        loadedConfig.m_ProjectFolder = "/local/machine/proj";
        loadedConfig.m_InitialScenePath = "/local/machine/proj/local.scene";
        Serialization::SceneSerializer reader;
        CHECK(reader.Deserialize(path, loaded, loadedConfig, format));
        VerifyScene(loaded, loadedConfig);

        std::cout << ((format == SerializationFormat::JSON) ? "JSON" : "Binary")
                  << " round-trip done\n";
        std::remove(path);
    }

    {
        const std::string projDir = "/tmp/nproj_save";
        const std::string sceneFile = projDir + "/scenes/s.scene";
        std::filesystem::create_directories(projDir + "/scenes");

        Engine::Registry reg;
        RegisterAll(reg);
        Serialization::SceneConfig config;
        config.m_ProjectFolder = projDir;
        config.m_InitialScenePath = sceneFile;
        Serialization::SceneSerializer writer;
        writer.Serialize(sceneFile, reg, config, SerializationFormat::JSON);

        std::ifstream in(sceneFile);
        const std::string raw((std::istreambuf_iterator<char>(in)),
                              std::istreambuf_iterator<char>());
        CHECK(raw.find("\"projectFolder\":\"..\"") != std::string::npos);
        CHECK(raw.find("\"initialScenePath\":\"scenes/s.scene\"") != std::string::npos);
        CHECK(raw.find("/tmp/nproj_save") == std::string::npos);

        Engine::Registry loaded;
        RegisterAll(loaded);
        Serialization::SceneConfig loadedConfig;
        Serialization::SceneSerializer reader;
        CHECK(reader.Deserialize(sceneFile, loaded, std::nullopt, SerializationFormat::JSON));
        CHECK(loaded.GetResource<Engine::ProjectContext>().projectFolder == projDir);

        std::filesystem::remove_all(projDir);
    }

    TestShapeColorKey();
    TestRTypeFilesLoad();

    if (failures == 0) {
        std::cout << "ALL CHECKS PASSED\n";
        return 0;
    }
    std::cerr << failures << " CHECKS FAILED\n";
    return 1;
}
