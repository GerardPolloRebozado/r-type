#include "Systems/TransformSystem.hpp"
#include "Components/Transform.hpp"

namespace Engine {
void TransformSystem::Update(Registry& registry, float dt) {
    for (auto const& entity : m_Entities) {
        auto& transform = registry.GetComponent<Transform>(entity);

        // TODO:
    }
}
} // namespace Engine
