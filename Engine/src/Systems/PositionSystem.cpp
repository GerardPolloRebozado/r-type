#include "Systems/PositionSystem.hpp"
#include "Components/Position.hpp"

namespace Engine {
void PositionSystem::Update(Registry& registry, float dt) {
    (void)dt;

    // Snapshot EVERY Position holder, not just System::m_Entities (which
    // nothing populates automatically). HierarchySystem computes parent
    // motion as (x - prevX), so a missed snapshot turns into a phantom delta
    // that flings relative children across the scene every frame.
    if (!registry.IsComponentRegistered<Position>()) {
        return;
    }
    auto positions = registry.GetComponentArray<Position>();
    for (auto& entry : *positions) {
        Position& position = entry.second;
        position.moved = position.x != position.prevX || position.y != position.prevY;
        position.prevX = position.x;
        position.prevY = position.y;
    }
}
} // namespace Engine
