#include "Systems/PositionSystem.hpp"
#include "Components/Position.hpp"

namespace Engine {
void PositionSystem::Update(Registry& registry, float dt) {
    (void)dt;

    for (Entity const entity : m_Entities) {
        if (!registry.HasComponent<Position>(entity)) {
            continue;
        }

        Position& position = registry.GetComponent<Position>(entity);
        position.moved = position.x != position.prevX || position.y != position.prevY ||
                         position.z != position.prevZ;
        position.prevX = position.x;
        position.prevY = position.y;
        position.prevZ = position.z;
    }
}
} // namespace Engine
