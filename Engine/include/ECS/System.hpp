#ifndef ENGINE_ECS_SYSTEM_HPP_
#define ENGINE_ECS_SYSTEM_HPP_
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
class Registry;

class System {
  public:
    virtual ~System() = default;
    virtual void Update(Registry& registry, float dt) {}
    virtual void Render(Registry& registry) {}

    std::set<Entity> m_Entities;
};
} // namespace Engine

#endif // ENGINE_ECS_SYSTEM_HPP_
