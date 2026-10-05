#ifndef ENGINE_COMPONENTS_TAGS_HPP_
#define ENGINE_COMPONENTS_TAGS_HPP_

#include <string>
#include <string_view>
#include <vector>

namespace Engine {
/**
 * @namespace Tag
 * @brief Built-in tag names used by most games.
 *
 * A custom tag is any other string passed to @ref TagSystem::Add. An entity
 * may carry several tags at once, built-in and custom together.
 */
namespace Tag {
inline constexpr std::string_view Player = "Player";
inline constexpr std::string_view Human = "Human";
inline constexpr std::string_view Ground = "Ground";
inline constexpr std::string_view Wall = "Wall";
} // namespace Tag

/**
 * @struct Tags
 * @brief Names that describe what an entity is.
 *
 * @ref Registry::CreateEntity attaches an empty list. Game code fills it
 * through @ref TagSystem. Presence in @ref values means the tag is set.
 */
struct Tags {
    /**
     * @brief Tag names currently set on this entity.
     *
     * Built-in names live in @ref Tag. Any other string is a custom tag.
     */
    std::vector<std::string> values;
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_TAGS_HPP_
