#include "Systems/TransformSystem.hpp"
#include "Components/Transform.hpp"

namespace Engine {
void TransformSystem::Update(Registry& registry, float dt) {
    auto array = registry.GetComponentArray<Transform>();
}
} // namespace Engine
