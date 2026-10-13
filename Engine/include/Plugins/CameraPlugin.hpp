#ifndef ENGINE_PLUGINS_CAMERAPLUGIN_HPP_
#define ENGINE_PLUGINS_CAMERAPLUGIN_HPP_

#include "Components/CameraComponent.hpp"
#include "Components/Position.hpp"
#include "Plugins/IPlugin.hpp"
#include "Systems/CameraSystem.hpp"

namespace Engine {

/**
 * @class CameraPlugin
 * @brief Plugin that bundles the CameraComponent and CameraSystem.
 */
class CameraPlugin : public IPlugin {
  public:
    /**
     * @brief Initializes the CameraPlugin by registering the CameraComponent and CameraSystem.
     * Also registers Position, which the camera uses as the point it looks at.
     * @param registry The ECS registry.
     */
    void Init(Registry& registry) override;

    /**
     * @brief Attaches a CameraComponent to an entity and ensures it has a Position.
     * An existing Position is left untouched.
     * @param registry The ECS registry.
     * @param entity The entity to attach the camera to.
     * @param camera The camera component to attach.
     */
    static void AttachToEntity(Registry& registry, Entity entity, const CameraComponent& camera);
};

} // namespace Engine

#endif // ENGINE_PLUGINS_CAMERAPLUGIN_HPP_
