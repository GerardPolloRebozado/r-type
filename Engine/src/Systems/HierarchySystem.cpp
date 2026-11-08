#include "Systems/HierarchySystem.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Logging/Logger.hpp"
#include <unordered_set>

namespace Engine {
void HierarchySystem::CollectReferencedParents(Registry& registry,
                                               std::unordered_set<Entity>& referencedAsParent) {
    auto parentArray = registry.GetComponentArray<Parent>();

    for (auto const& entry : *parentArray) {
        referencedAsParent.insert(entry.second.parent);
    }
}

void HierarchySystem::Update(Registry& registry, float dt) {
    (void)dt;

    // One walk per forest root avoids double-applying deltas on deep trees.
    std::vector<Entity> const roots = FindHierarchyRoots(registry);

    for (Entity const root : roots) {
        WalkSubtree(registry, root);
    }
}

std::vector<Entity> HierarchySystem::FindHierarchyRoots(Registry& registry) {
    std::unordered_set<Entity> referencedAsParent;
    std::vector<Entity> roots;

    CollectReferencedParents(registry, referencedAsParent);

    // Roots = referenced as someone's parent but not parented themselves (top of each tree).
    for (Entity const candidate : referencedAsParent) {
        if (!registry.HasComponent<Parent>(candidate)) {
            roots.push_back(candidate);
        }
    }

    return roots;
}

std::vector<Entity> HierarchySystem::FindDirectChildren(Registry& registry, Entity node) {
    auto parentArray = registry.GetComponentArray<Parent>();
    std::vector<Entity> children;

    children.reserve(parentArray->size());

    for (auto const& entry : *parentArray) {
        if (entry.second.parent == node) {
            children.push_back(entry.first);
        }
    }

    return children;
}

void HierarchySystem::TeleportWithChildren(Registry& registry, Entity entity, float x, float y) {
    if (!registry.HasComponent<Position>(entity)) {
        return;
    }
    Position& position = registry.RequireComponent<Position>(entity);
    float const dx = x - position.x;
    float const dy = y - position.y;
    position.x = x;
    position.y = y;
    position.prevX = x;
    position.prevY = y;
    position.moved = true;

    if (registry.IsComponentRegistered<Parent>() && (dx != 0.0f || dy != 0.0f)) {
        ShiftRelativeDescendants(registry, entity, dx, dy);
    }
}

void HierarchySystem::ShiftRelativeDescendants(Registry& registry, Entity node, float dx,
                                               float dy) {
    for (Entity const child : FindDirectChildren(registry, node)) {
        if (!registry.HasComponent<Position>(child)) {
            continue;
        }
        Position& childPosition = registry.RequireComponent<Position>(child);
        // Same rule as WalkSubtree: only relative children follow their parent.
        if (!childPosition.relative) {
            continue;
        }
        childPosition.x += dx;
        childPosition.y += dy;
        childPosition.prevX += dx;
        childPosition.prevY += dy;
        ShiftRelativeDescendants(registry, child, dx, dy);
    }
}

void HierarchySystem::WalkSubtree(Registry& registry, Entity node) {
    if (!registry.HasComponent<Position>(node)) {
        log_error("HierarchySystem: entity " + std::to_string(node) + " has no Position");
        return;
    }

    Position& nodePosition = registry.RequireComponent<Position>(node);
    // x/y/z are world space; delta comes from PositionSystem's prev snapshot on the prior frame.
    float const dx = nodePosition.x - nodePosition.prevX;
    float const dy = nodePosition.y - nodePosition.prevY;
    int const dz = nodePosition.z - nodePosition.prevZ;

    std::vector<Entity> const children = FindDirectChildren(registry, node);

    for (Entity const child : children) {
        if (registry.HasComponent<Position>(child)) {
            Position& childPosition = registry.RequireComponent<Position>(child);

            if (childPosition.relative) {
                childPosition.x += dx;
                childPosition.y += dy;
                childPosition.z += dz;
            }
        }

        // Always recurse: a middle node can move while its ancestor's delta is zero.
        WalkSubtree(registry, child);
    }
}
} // namespace Engine
