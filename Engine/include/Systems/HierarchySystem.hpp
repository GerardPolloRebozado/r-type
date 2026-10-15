#ifndef ENGINE_SYSTEMS_HIERARCHYSYSTEM_HPP_
#define ENGINE_SYSTEMS_HIERARCHYSYSTEM_HPP_

#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "ECS/System.hpp"
#include <unordered_set>
#include <vector>

namespace Engine {
/**
 * @class HierarchySystem
 * @brief Propagates parent @ref Position deltas to relative children each frame.
 *
 * Discovers hierarchy from @ref Parent components only (no stored child lists).
 * Runs one subtree walk per hierarchy root (entity referenced as parent with no
 * @ref Parent of its own). At each node, applies that node's frame delta to direct
 * children with @ref Position::relative set, then recurses regardless of zero delta.
 *
 * @note Must run before @ref PositionSystem in the registry update order so child
 *       positions are updated before prevX/prevY/prevZ are snapshotted.
 */
class HierarchySystem : public System {
  public:
    /**
     * @brief Applies parent motion to relative children for all hierarchy roots.
     * @param registry ECS registry with @ref Parent and @ref Position registered.
     * @param dt Delta time (unused; kept for @ref System API).
     */
    void Update(Registry& registry, float dt) override;

  private:
    static void CollectReferencedParents(Registry& registry,
                                         std::unordered_set<Entity>& referencedAsParent);

    /**
     * @brief Returns hierarchy roots (referenced as parent, with no @ref Parent of their own).
     */
    static std::vector<Entity> FindHierarchyRoots(Registry& registry);

    /**
     * @brief Returns entities whose @ref Parent::parent equals @p node.
     */
    static std::vector<Entity> FindDirectChildren(Registry& registry, Entity node);

    /**
     * @brief Computes this node's delta and propagates to relative children, then recurses.
     */
    static void WalkSubtree(Registry& registry, Entity node);
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_HIERARCHYSYSTEM_HPP_
