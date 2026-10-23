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
    /**
     * @brief Called right before the current scene's entities are destroyed.
     *
     * Override it to drop state kept outside components (script instances,
     * playing sounds, cached pairs...). Entities are still alive during this
     * call; after it, entity IDs restart at 1.
     */
    virtual void OnSceneUnloaded(Registry& registry) {}

    std::set<Entity> m_Entities;
};
} // namespace Engine

#endif // ENGINE_ECS_SYSTEM_HPP_
