#ifndef ENGINE_SYSTEMS_TAGSYSTEM_HPP_
#define ENGINE_SYSTEMS_TAGSYSTEM_HPP_

#include "Components/Tags.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "ECS/System.hpp"
#include <string_view>
#include <vector>

namespace Engine {
/**
 * @class TagSystem
 * @brief Reads and edits @ref Tags. Does not simulate anything each frame.
 *
 * Tags are data. This system is the place that adds a name, removes one, or
 * answers whether an entity has it. Built-in names come from @ref Tag. A
 * custom tag is any other string, for example `"Boss"`.
 *
 * Queries scan every id returned by @ref Registry::GetEntityCount and skip
 * entities whose @ref Tags component was removed (including destroyed ones).
 * Callers do not insert entities into @ref System::m_Entities.
 */
class TagSystem : public System {
  public:
    /**
     * @brief Appends @p tag when it is not already present.
     * @param tag Built-in name from @ref Tag, or any custom string.
     */
    static void Add(Tags& tags, std::string_view tag);

    /**
     * @brief Appends @p tag on @p entity when it is not already present.
     * @param tag Built-in name from @ref Tag, or any custom string.
     */
    static void Add(Registry& registry, Entity entity, std::string_view tag);

    /**
     * @brief Removes @p tag when it is present.
     */
    static void Remove(Tags& tags, std::string_view tag);

    /**
     * @brief Removes @p tag from @p entity when it is present.
     */
    static void Remove(Registry& registry, Entity entity, std::string_view tag);

    /**
     * @brief True when @p tags contains @p tag.
     */
    static bool Has(const Tags& tags, std::string_view tag);

    /**
     * @brief True when @p entity has @p tag.
     * @return False when the entity has no @ref Tags component or the name is absent.
     */
    static bool Has(Registry& registry, Entity entity, std::string_view tag);

    /**
     * @brief Every live entity whose @ref Tags contain @p tag.
     * @param tag Built-in name from @ref Tag, or any custom string.
     * @return Matching entity ids. Empty when nobody has that tag.
     */
    static std::vector<Entity> EntitiesWithTag(Registry& registry, std::string_view tag);
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_TAGSYSTEM_HPP_
