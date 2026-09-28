#pragma once
#include "Entity.hpp"
#include <set>

namespace Engine {
/**
 * @class System
 * @brief Base class for all systems in the engine.
 *
 * @note Systems exist globally and automatically process any entity that
 * possesses the data (Components) required by that system.
 */
class System {
  public:
    /**
     * @brief The set of entities managed by this system.
     */
    std::set<Entity> m_Entities;
};
} // namespace Engine
