#include "Assets/ProjectPaths.hpp"
#include "Components/AudioSource.hpp"
#include "Core/Application.hpp"
#include "Plugins/AudioPlugin.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/RotationPlugin.hpp"
#include "Scripting/ScriptPlugin.hpp"
#include "UI/UiComponent.hpp"
#include "UI/UiPlugin.hpp"
#include <string>

#ifndef MENU_SOURCE_DIR
#define MENU_SOURCE_DIR "."
#endif

int main() {
    Engine::ApplicationConfig config;
    Engine::Entity menu = 0;
    Engine::Entity music = 0;
    Engine::Entity effect = 0;
    Engine::AudioSource title;
    Engine::AudioSource explosion;

    config.windowTitle = "UI Menu";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.targetFPS = 60;

    Engine::Application app(config);
    auto& registry = app.GetRegistry();

    registry.AddResource<Engine::ProjectContext>(std::string(MENU_SOURCE_DIR));
    // CreateEntity always adds Position and Rotation, so those plugins have to be present.
    Engine::PositionPlugin().Init(registry);
    Engine::RotationPlugin().Init(registry);
    Engine::ScriptPlugin().Init(registry);
    Engine::UiPlugin().Init(registry);
    Engine::AudioPlugin().Init(registry);

    menu = registry.CreateEntity();
    Engine::UiPlugin::AttachToEntity(registry, menu, Engine::UiComponent("ui/main_menu.lua"));

    // R-Type tracks, so the settings volume and mute rows have something to change.
    // Paths are relative to Games/Menu, which is this project's folder.
    music = registry.CreateEntity();
    title.assetId = "../../R-Type/assets/audio/title.mp3";
    title.isMusic = true;
    title.shouldPlay = true;
    title.loop = true;
    title.volume = 1.0f;
    registry.AddComponent(music, title);

    effect = registry.CreateEntity();
    explosion.assetId = "../../R-Type/assets/audio/explosion.mp3";
    explosion.isMusic = false;
    explosion.shouldPlay = true;
    explosion.loop = true;
    explosion.volume = 0.35f;
    registry.AddComponent(effect, explosion);

    app.Run();
    return 0;
}
