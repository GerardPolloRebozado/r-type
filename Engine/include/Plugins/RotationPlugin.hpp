#ifndef ENGINE_PLUGINS_ROTATIONPLUGIN_HPP_
#define ENGINE_PLUGINS_ROTATIONPLUGIN_HPP_

#include "Components/Rotation.hpp"
#include "Plugins/IPlugin.hpp"

/**
 * @file RotationPlugin.hpp
 * @brief Declares the plugin that registers the Rotation component.
 */
namespace Engine {

/**
 * @class RotationPlugin
 * @brief Plugin that registers the Rotation component with the registry.
 */
class RotationPlugin : public IPlugin {
  public:
    /**
     * @brief Registers the Rotation component and its system.
     * @param registry Registry receiving the component/system.
     */
    void Init(Registry& registry) override;

    /**
     * @brief Attaches a Rotation component to an entity.
     * @param registry Registry containing the entity.
     * @param entity Entity receiving the component.
     * @param angle Initial angle in degrees.
     */
    static void AttachToEntity(Registry& registry, Entity entity, float angle = 0.0f);
};

} // namespace Engine

#endif // ENGINE_PLUGINS_ROTATIONPLUGIN_HPP_