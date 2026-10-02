#include "Components/Position.hpp"
#include "ECS/Registry.hpp"
#include "Plugins/TransformPlugin.hpp"
#include <iostream>

int main() {
    Engine::Registry registry;
    Engine::TransformPlugin transformPlugin;
    transformPlugin.Init(registry);

    Engine::Entity player = registry.CreateEntity();
    std::cout << "Spawned Player Entity ID: " << player << "\n";

    registry.AddComponent(player, Engine::Position{0.0f, 0.0f});

    auto transformSystem = registry.GetSystem<Engine::TransformSystem>();
    transformSystem->m_Entities.insert(player);

    auto printPos = [&]() {
        auto& position = registry.GetComponent<Engine::Position>(player);
        std::cout << "Player position is now: (" << position.x << ", " << position.y << ")\n";
    };

    printPos();

    std::cout << "\n> Moving 10 steps right...\n";
    transformSystem->Update(registry, 1.0f);
    registry.GetComponent<Engine::Position>(player).x += 10.0f;
    printPos();

    std::cout << "\n> Moving 20 steps bottom...\n";
    transformSystem->Update(registry, 1.0f);
    registry.GetComponent<Engine::Position>(player).y -= 20.0f;
    printPos();

    std::cout << "\n> Moving 10 steps left...\n";
    transformSystem->Update(registry, 1.0f);
    registry.GetComponent<Engine::Position>(player).x -= 40.0f;
    printPos();

    std::cout << "\n> Moving 10 steps up...\n";
    transformSystem->Update(registry, 1.0f);
    registry.GetComponent<Engine::Position>(player).y += 15.0f;
    printPos();
    return 0;
}
