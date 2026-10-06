#include "BundledAssets.hpp"
#include "Core/Application.hpp"
#include "Plugins/AudioPlugin.hpp"
#include <iostream>

int main() {
    std::cout << "======================================\n";
    std::cout << "           ECS Audio Sandbox          \n";
    std::cout << "======================================\n";

    Engine::ApplicationConfig config;
    config.windowTitle = "Audio Sandbox";
    config.targetFPS = 60;
    ChangeDirectory(GetApplicationDirectory());

    Engine::Application app(config);

    Engine::AudioPlugin audioPlugin;
    audioPlugin.Init(app.GetRegistry());

    auto& assets = app.GetRegistry().GetResource<Engine::AssetManager>();
    BundledAssets::RegisterAssets(assets);

    if (assets.GetAllMusic().contains("music.mp3")) {
        assets.GetMusic("music.mp3")->SetLooping(true);
    }

    const Engine::Entity playerEntity = app.GetRegistry().CreateEntity();

    Engine::AudioSource audioData;
    audioData.assetId = "music.mp3";
    audioData.isMusic = true;
    audioData.shouldPlay = true;

    app.GetRegistry().AddComponent(playerEntity, audioData);

    app.GetRegistry().GetSystem<Engine::AudioSystem>()->m_Entities.insert(playerEntity);

    app.Run();

    return 0;
}
