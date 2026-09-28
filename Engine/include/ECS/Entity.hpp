#pragma once
#include <cstddef>

namespace Engine {
/**
 * @brief Type definition for an Entity.
 */
using Entity = std::size_t;

// TODO: entity limit? optimizations when a large amount of entities are in the loop?
// reallocation for map/vector sizes can cause lag if a large amount is used

} // namespace Engine
