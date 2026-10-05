#ifndef ENGINE_PLUGINS_COLLISIONPLUGIN_HPP_
#define ENGINE_PLUGINS_COLLISIONPLUGIN_HPP_

#include "Components/Collider.hpp"
#include "Components/CollisionEvents.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Plugins/IPlugin.hpp"
#include "Systems/CollisionSystem.hpp"

namespace Engine {
/**
 * @class CollisionPlugin
 * @brief Bundles collider data, collision events, and @ref CollisionSystem.
 *
 * Does not register @ref Position; entities need a position (e.g. via
 * @ref PositionPlugin) for world-space overlap tests.
 */
class CollisionPlugin : public IPlugin {
  public:
    /**
     * @brief Registers collider-related components and the collision system.
     * @param registry Main ECS registry.
     */
    void Init(Registry& registry) override {
        registry.RegisterComponent<Collider>();
        registry.RegisterComponent<CollisionEvents>();
        registry.RegisterSystem<CollisionSystem>();
    }

    /**
     * @brief Attaches default collider and event hooks to an entity.
     * @param registry Main ECS registry.
     * @param entity Target entity (caller must register it with @ref CollisionSystem::m_Entities).
     * @param width Initial collider width.
     * @param height Initial collider height.
     */
    static void AttachToEntity(Registry& registry, Entity entity, float width = 1.0f,
                               float height = 1.0f) {
        registry.AddComponent(entity, Collider{width, height});
        registry.AddComponent(entity, CollisionEvents{});
    }
};
} // namespace Engine

#endif // ENGINE_PLUGINS_COLLISIONPLUGIN_HPP_
