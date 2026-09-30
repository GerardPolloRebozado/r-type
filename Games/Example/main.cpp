#include "Core/Application.hpp"
#include "Plugins/PluginExample.hpp"
#include <iostream>

#include "Plugins/TextPlugin.hpp"
#include "Plugins/TransformPlugin.hpp"

int main() {
    std::cout << "======================================\n";
    std::cout << "      Starting Sandbox Game Demo      \n";
    std::cout << "======================================\n";

    Engine::ApplicationConfig config;
    config.windowTitle = "Example Sandbox Game";
    config.targetFPS = 60;

    Engine::Application app(config);

    Engine::PluginExample plugin;
    Engine::TextPlugin textPlugin;
    Engine::TransformPlugin tform;

    tform.Init(app.GetRegistry());
    textPlugin.Init(app.GetRegistry());

    plugin.Init(app.GetRegistry());

    Engine::Entity myEntity = app.GetRegistry().CreateEntity();
    std::cout << "Spawned test entity: " << myEntity << "\n";

    Engine::PluginExample::AttachToEntity(app.GetRegistry(), myEntity);

    auto& events = app.GetRegistry().GetComponent<Engine::PluginExampleEvents>(myEntity);
    events.onStateChanged.push_back([&app](Engine::Entity me, int newState) {
        std::cout << "[EVENT FIRED] Entity " << me << " state changed to: " << newState << "\n";

        if (newState >= 3) {
            std::cout << "[GAME LOGIC] State reached 3. Queuing entity destruction and shutting "
                         "down the engine safely!\n";

            app.GetRegistry().QueueDestroyEntity(me);
            app.Quit();
        }
    });

    app.GetRegistry().GetSystem<Engine::SystemExample>()->m_Entities.insert(myEntity);
    //create the entity for a text on the screen
    Engine::Entity mytextEntity = app.GetRegistry().CreateEntity();
    app.GetRegistry().AddComponent(mytextEntity, Engine::Transform{100.0f, 100.0f});
    Engine::TextPlugin::AttachToEntity(app.GetRegistry(), mytextEntity, "Hello, World!");

    app.Run();

    return 0;
}
