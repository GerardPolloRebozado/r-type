#include "Assets/ProjectPaths.hpp"
#include "Components/AudioSource.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/CollisionEvents.hpp"
#include "Components/Input.hpp"
#include "Components/Name.hpp"
#include "Components/Opacity.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "Components/SignalConnections.hpp"
#include "Components/SpriteComponent.hpp"
#include "Components/Text.hpp"
#include "ECS/Registry.hpp"
#include "SceneConfig.hpp"
#include "SceneSerializer.hpp"
#include "Scripting/ScriptComponent.hpp"
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>

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
        if (reg.HasComponent<Engine::Name>(i) && reg.GetComponent<Engine::Name>(i).value == name) {
            return i;
        }
    }
    return static_cast<Engine::Entity>(-1);
}

static void VerifyScene(Engine::Registry& reg, const Serialization::SceneConfig& config) {
    CHECK(reg.GetEntityCount() == 4); // 1 initial slot + 3 created entities

    Engine::Entity player = FindByName(reg, "Player");
    Engine::Entity enemy = FindByName(reg, "Enemy");
    CHECK(player != static_cast<Engine::Entity>(-1));
    CHECK(enemy != static_cast<Engine::Entity>(-1));

    const auto& pos = reg.GetComponent<Engine::Position>(player);
    CHECK(pos.x == 10.0f);
    CHECK(pos.y == 20.0f);
    CHECK(reg.GetComponent<Engine::Rotation>(player).angle == 45.0f);

    const auto& shape = reg.GetComponent<Engine::ShapeComponent>(player);
    CHECK(shape.type == Engine::ShapeType::RECTANGLE);
    CHECK(shape.width == 100.0f);
    CHECK(shape.height == 50.0f);
    CHECK(shape.color.r == 10);
    CHECK(shape.color.g == 20);
    CHECK(shape.color.b == 30);
    CHECK(shape.color.a == 40);

    const auto& text = reg.GetComponent<Engine::Text>(player);
    CHECK(text.text == "hello");
    CHECK(text.fontsize == 12.0f);
    CHECK(text.color.r == 1);
    CHECK(text.color.a == 4);

    const auto& input = reg.GetComponent<Engine::Input>(player);
    CHECK(input.playerIdx == 2);
    CHECK(input.isActive == false);

    const auto& audio = reg.GetComponent<Engine::AudioSource>(player);
    CHECK(audio.assetId == "sfx/laser.wav");
    CHECK(audio.volume == 0.5f);
    CHECK(audio.loop == true);

    const auto& ecol = reg.GetComponent<Engine::ColliderComponent>(enemy);
    CHECK(ecol.enabled == true);
    CHECK(ecol.isTrigger == true);
    CHECK(ecol.shape.has_value());
    CHECK(ecol.shape->type == Engine::ShapeType::CIRCLE);
    CHECK(ecol.shape->radius == 10.0f);
    CHECK(reg.GetComponent<Engine::Parent>(enemy).parent == player);

    // Unnamed sprite entity: no Name component
    bool foundSprite = false;
    for (std::size_t i = 0; i < reg.GetEntityCount(); ++i) {
        if (!reg.HasComponent<Engine::Name>(i) && reg.HasComponent<Engine::SpriteComponent>(i)) {
            const auto& spr = reg.GetComponent<Engine::SpriteComponent>(i);
            CHECK(spr.assetPath == "assets/ship.png");
            CHECK(spr.offsetX == 5.0f);
            CHECK(spr.offsetY == 6.0f);
            CHECK(spr.scaleX == 2.0f);
            CHECK(spr.scaleY == 3.0f);
            CHECK(reg.GetComponent<Engine::Rotation>(i).angle == 90.0f);
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

    if (failures == 0) {
        std::cout << "ALL CHECKS PASSED\n";
        return 0;
    }
    std::cerr << failures << " CHECKS FAILED\n";
    return 1;
}
