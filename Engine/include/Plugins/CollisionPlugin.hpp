#ifndef ENGINE_PLUGINS_COLLISIONPLUGIN_HPP_
#define ENGINE_PLUGINS_COLLISIONPLUGIN_HPP_

#include "Components/Collider.hpp"
#include "Components/CollisionEvents.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Plugins/IPlugin.hpp"
#include "Systems/CollisionSystem.hpp"
#include <raylib.h>

namespace Engine {
/**
 * @class CollisionPlugin
 * @brief Bundles collider data, collision events, and @ref CollisionSystem.
 */
class CollisionPlugin : public IPlugin {
  public:
    /**
     * @brief Registers collider-related components, Position, and the collision system.
     * @param registry Main ECS registry.
     */
    void Init(Registry& registry) override;

    /**
     * @brief Attaches collider and event hooks to an entity.
     * @param registry Main ECS registry.
     * @param entity Target entity.
     * @param shape Optional shape that will be used to designate the area
     * @param enabled Whether collision queries include this collider.
     * @param isTrigger Trigger vs solid semantics.
     */
    static void AttachToEntity(Registry& registry, Entity entity,
                               const std::optional<ShapeComponent>& shape = std::nullopt,
                               bool enabled = true, bool isTrigger = false);
};
} // namespace Engine

#endif // ENGINE_PLUGINS_COLLISIONPLUGIN_HPP_