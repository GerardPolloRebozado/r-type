#include "Components/Transform.hpp"
#include "ECS/Registry.hpp"
#include "Plugins/TransformPlugin.hpp"
#include <iostream>

int main() {
    Engine::Registry registry;
    Engine::TransformPlugin transformPlugin;
    transformPlugin.Init(registry);

    Engine::Entity player = registry.CreateEntity();
    std::cout << "Spawned Player Entity ID: " << player << "\n";

    registry.AddComponent(player, Engine::Transform{0.0f, 0.0f});

    auto transformSystem = registry.GetSystem<Engine::TransformSystem>();

    auto printPos = [&]() {
        auto& t = registry.GetComponent<Engine::Transform>(player);
        std::cout << "Player position is now: (" << t.x << ", " << t.y << ")\n";
    };

    printPos();

    std::cout << "\n> Moving 10 steps right...\n";
    registry.GetComponent<Engine::Transform>(player).x += 10.0f;
    transformSystem->Update(registry, 1.0f);
    printPos();

    std::cout << "\n> Moving 20 steps bottom...\n";
    registry.GetComponent<Engine::Transform>(player).y -= 20.0f;
    transformSystem->Update(registry, 1.0f);
    printPos();

    std::cout << "\n> Moving 10 steps left...\n";
    registry.GetComponent<Engine::Transform>(player).x -= 40.0f;
    transformSystem->Update(registry, 1.0f);
    printPos();

    std::cout << "\n> Moving 10 steps up...\n";
    registry.GetComponent<Engine::Transform>(player).y += 15.0f;
    transformSystem->Update(registry, 1.0f);
    printPos();
    return 0;
}
