#ifndef ENGINE_COMPONENTS_NAME_HPP_
#define ENGINE_COMPONENTS_NAME_HPP_

#include "ECS/Registry.hpp"
#include <string>
#include <utility>

namespace Engine {
/**
 * @struct Name
 * @brief A human-readable name for an entity (e.g. "Player", "Enemy").
 *
 * Only attached to entities that have been given a name; unnamed entities
 * simply don't have this component. Registered by @ref Application.
 */
struct Name {
    std::string value;

    /**
     * @brief Creates a Name component.
     * @param value The name of the entity.
     */
    static Name Create(std::string value = "") {
        Name name;
        name.value = std::move(value);
        return name;
    }
};

/**
 * @brief Returns the display name of an entity.
 * @param registry The registry holding the entity.
 * @param entity The entity for which to get the name.
 * @return The entity's @ref Name, or "Entity X" if it has none.
 */
inline std::string GetEntityName(Registry& registry, Entity entity) {
    if (registry.HasComponent<Name>(entity)) {
        return registry.RequireComponent<Name>(entity).value;
    }
    return "Entity " + std::to_string(entity);
}
} // namespace Engine

#endif // ENGINE_COMPONENTS_NAME_HPP_
