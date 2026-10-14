#include "Components/Position.hpp"
#include "Plugins/HierarchyPlugin.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Systems/HierarchySystem.hpp"
#include "Systems/PositionSystem.hpp"
#include <cmath>
#include <iostream>

namespace {
bool NearlyEqual(float a, float b) {
    return std::fabs(a - b) < 0.0001f;
}

bool RunHierarchySmokeTest() {
    Engine::Registry registry;
    Engine::HierarchyPlugin hierarchyPlugin;
    Engine::PositionPlugin positionPlugin;

    hierarchyPlugin.Init(registry);
    positionPlugin.Init(registry);

    Engine::Entity parent = registry.CreateEntity();
    Engine::Entity child = registry.CreateEntity();

    registry.AddComponent(parent, Engine::Position::Create(100.0f, 200.0f));
    registry.AddComponent(child, Engine::Position::Create(110.0f, 220.0f));
    Engine::HierarchyPlugin::AttachChild(registry, child, parent, true);

    registry.GetSystem<Engine::PositionSystem>()->Update(registry, 0.0f);

    Engine::Position& parentPos = registry.GetComponent<Engine::Position>(parent);
    parentPos.x = 150.0f;

    registry.GetSystem<Engine::HierarchySystem>()->Update(registry, 0.0f);
    registry.GetSystem<Engine::PositionSystem>()->Update(registry, 0.0f);

    Engine::Position const& childPos = registry.GetComponent<Engine::Position>(child);
    if (!NearlyEqual(childPos.x, 160.0f) || !NearlyEqual(childPos.y, 220.0f)) {
        std::cerr << "[FAIL] relative child did not follow parent delta (got " << childPos.x << ", "
                  << childPos.y << ")\n";
        return false;
    }

    registry.DestroyEntity(parent);

    if (registry.HasComponent<Engine::Parent>(child) ||
        registry.HasComponent<Engine::Position>(child)) {
        std::cerr << "[FAIL] cascade destroy left child components\n";
        return false;
    }

    std::cout << "[PASS] hierarchy follow + cascade destroy\n";
    return true;
}
} // namespace

int main() {
    std::cout << "======================================\n";
    std::cout << "      Hierarchy Engine Smoke Test     \n";
    std::cout << "======================================\n";

    return RunHierarchySmokeTest() ? 0 : 1;
}
