#include "Components/Animation.hpp"
#include "Components/Position.hpp"
#include "Core/Application.hpp"
#include "Plugins/AnimationPlugin.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/TextPlugin.hpp"
#include "Resources/ProjectContext.hpp"
#include <filesystem>
#include <iostream>

#ifndef ASSETS_DIR
#define ASSETS_DIR "./"
#endif

int main() {
    std::cout << "======================================\n";
    std::cout << "      Starting Animation Demo Game    \n";
    std::cout << "======================================\n";

    Engine::ApplicationConfig config;
    config.windowTitle = "Animation Demo";
    config.windowWidth = 800;
    config.windowHeight = 600;
    config.targetFPS = 60;

    Engine::Application app(config);

    std::string baseDir = ASSETS_DIR;
    if (!std::filesystem::exists(baseDir + "enemy1.png")) {
        if (std::filesystem::exists("Games/Animations/enemy1.png")) {
            baseDir = "Games/Animations/";
        } else if (std::filesystem::exists("../Games/Animations/enemy1.png")) {
            baseDir = "../Games/Animations/";
        }
    }
    app.GetRegistry().AddResource<Engine::ProjectContext>(baseDir);

    Engine::PositionPlugin positionPlugin;
    positionPlugin.Init(app.GetRegistry());

    Engine::TextPlugin textPlugin;
    textPlugin.Init(app.GetRegistry());

    Engine::AnimationPlugin animationPlugin;
    animationPlugin.Init(app.GetRegistry());

    Engine::Entity titleEntity = app.GetRegistry().CreateEntity();
    app.GetRegistry().AddComponent(titleEntity, Engine::Position(50.0f, 30.0f));
    Engine::TextPlugin::AttachToEntity(app.GetRegistry(), titleEntity, "Animation System Demo");

    Engine::Entity labelUniform = app.GetRegistry().CreateEntity();
    app.GetRegistry().AddComponent(labelUniform, Engine::Position(50.0f, 100.0f));
    Engine::TextPlugin::AttachToEntity(app.GetRegistry(), labelUniform,
                                       "Uniform Spritesheet (enemy1.png):");

    Engine::Entity labelCustom = app.GetRegistry().CreateEntity();
    app.GetRegistry().AddComponent(labelCustom, Engine::Position(50.0f, 260.0f));
    Engine::TextPlugin::AttachToEntity(app.GetRegistry(), labelCustom,
                                       "Custom FrameData Slicing & Pivot:");

    Engine::Entity labelMulti = app.GetRegistry().CreateEntity();
    app.GetRegistry().AddComponent(labelMulti, Engine::Position(50.0f, 420.0f));
    Engine::TextPlugin::AttachToEntity(app.GetRegistry(), labelMulti,
                                       "Multi-file frames (red-spinner):");

    // Entity (enemy1.png has 192x24 -> 8 frames of 24x24)
    Engine::Entity uniformAnimEntity = app.GetRegistry().CreateEntity();
    app.GetRegistry().AddComponent(uniformAnimEntity, Engine::Position(80.0f, 150.0f));
    Engine::Animation uniformAnim("enemy1.png", 24.0f, 24.0f, 8, 0.1f, true, 0.0f, 0.0f, 2.5f,
                                  2.5f);
    Engine::AnimationPlugin::AttachToEntity(app.GetRegistry(), uniformAnimEntity, uniformAnim);

    Engine::Entity customAnimEntity = app.GetRegistry().CreateEntity();
    app.GetRegistry().AddComponent(customAnimEntity, Engine::Position(80.0f, 310.0f));

    std::vector<Engine::FrameData> customFrames = {
        Engine::FrameData(0.0f, 0.0f, 24.0f, 24.0f, 0.0f, 0.0f),
        Engine::FrameData(24.0f, 0.0f, 24.0f, 24.0f, 0.0f, -4.0f),
        Engine::FrameData(48.0f, 0.0f, 24.0f, 24.0f, 0.0f, -8.0f),
        Engine::FrameData(72.0f, 0.0f, 24.0f, 24.0f, 0.0f, -4.0f),
        Engine::FrameData(96.0f, 0.0f, 24.0f, 24.0f, 0.0f, 0.0f),
        Engine::FrameData(120.0f, 0.0f, 24.0f, 24.0f, 0.0f, 4.0f),
        Engine::FrameData(144.0f, 0.0f, 24.0f, 24.0f, 0.0f, 8.0f),
        Engine::FrameData(168.0f, 0.0f, 24.0f, 24.0f, 0.0f, 4.0f)};

    Engine::Animation customAnim("enemy1.png", customFrames, 0.12f, true, 0.0f, 0.0f, 2.5f, 2.5f);
    Engine::AnimationPlugin::AttachToEntity(app.GetRegistry(), customAnimEntity, customAnim);

    Engine::Entity multiAnimEntity = app.GetRegistry().CreateEntity();
    app.GetRegistry().AddComponent(multiAnimEntity, Engine::Position(80.0f, 470.0f));
    std::vector<std::string> multiFiles = {
        "assets/enemies/red-spinner/enemy1000.png", "assets/enemies/red-spinner/enemy1001.png",
        "assets/enemies/red-spinner/enemy1002.png", "assets/enemies/red-spinner/enemy1003.png",
        "assets/enemies/red-spinner/enemy1004.png", "assets/enemies/red-spinner/enemy1005.png",
        "assets/enemies/red-spinner/enemy1006.png", "assets/enemies/red-spinner/enemy1007.png"};
    Engine::Animation multiAnim(multiFiles, 0.08f, true, 0.0f, 0.0f, 2.5f, 2.5f);
    Engine::AnimationPlugin::AttachToEntity(app.GetRegistry(), multiAnimEntity, multiAnim);

    app.Run();

    return 0;
}
