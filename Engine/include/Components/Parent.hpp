#ifndef ENGINE_COMPONENTS_PARENT_HPP_
#define ENGINE_COMPONENTS_PARENT_HPP_

#include "ECS/Entity.hpp"

namespace Engine {
/**
 * @struct Parent
 * @brief Links an entity to its parent in a transform hierarchy.
 *
 * Used for grouped destroy (@ref Registry destroys the full subtree) and, with
 * @ref Position::relative, for following the parent's motion via @ref HierarchySystem.
 * Child world position stays in @ref Position; this component only stores the parent link.
 */
struct Parent {
    /** @brief Entity id of the parent in the same @ref Registry. */
    Entity parent = 0;

    /**
     * @brief Creates a Parent component.
     * @param parentEntity The parent entity id.
     */
    static Parent Create(Entity parentEntity = 0) {
        Parent p;
        p.parent = parentEntity;
        return p;
    }
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_PARENT_HPP_
