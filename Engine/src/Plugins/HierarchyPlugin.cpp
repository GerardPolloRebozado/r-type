#include "Plugins/HierarchyPlugin.hpp"
#include "Errors/EcsErrors.hpp"

namespace Engine {
void HierarchyPlugin::Init(Registry& registry) {
    registry.RegisterComponent<Parent>();
    registry.RegisterSystem<HierarchySystem>();
}

void HierarchyPlugin::AttachChild(Registry& registry, Entity child, Entity parent, bool relative) {
    // Self-parent and cycles would break subtree walks and cascade destroy.
    if (child == parent) {
        throw ErrorEcs("AttachChild: entity cannot be its own parent");
    }

    Entity ancestor = parent;

    while (registry.HasComponent<Parent>(ancestor)) {
        ancestor = registry.GetComponent<Parent>(ancestor).parent;
        if (ancestor == child) {
            throw ErrorEcs("AttachChild: would create a parent cycle");
        }
    }

    registry.AddComponent(child, Parent::Create(parent));

    if (relative) {
        if (!registry.HasComponent<Position>(child)) {
            throw ErrorEcs("AttachChild: relative child requires a Position component");
        }
        registry.GetComponent<Position>(child).relative = true;
    }
}
} // namespace Engine
