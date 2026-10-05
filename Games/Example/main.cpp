#include "Core/Application.hpp"
#include "Plugins/PluginExample.hpp"
#include <iostream>

#include "Plugins/PositionPlugin.hpp"
#include "Plugins/TextPlugin.hpp"

int main() {
    std::cout << "======================================\n";
    std::cout << "      Starting Sandbox Game Demo      \n";
    std::cout << "======================================\n";

    Engine::ApplicationConfig config;
    config.windowTitle = "Example Sandbox Game";
    config.targetFPS = 60;

    Engine::Application app(config);

    Engine::PluginExample plugin;

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
    // create the entity for a text on the screen

    app.Run();

    return 0;
}
