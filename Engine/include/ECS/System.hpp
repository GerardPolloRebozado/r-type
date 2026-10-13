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
    /** @brief World pass: drawn inside the active camera. */
    virtual void Render(Registry& registry) {}
    /** @brief Screen pass: drawn after the world, in window pixels. */
    virtual void RenderScreen(Registry& registry) {}

    std::set<Entity> m_Entities;
};
} // namespace Engine

#endif // ENGINE_ECS_SYSTEM_HPP_
