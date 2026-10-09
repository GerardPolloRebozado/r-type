#ifndef ENGINE_PLUGINS_ROTATIONPLUGIN_HPP_
#define ENGINE_PLUGINS_ROTATIONPLUGIN_HPP_

#include "Components/Rotation.hpp"
#include "Plugins/IPlugin.hpp"

namespace Engine {

class RotationPlugin : public IPlugin {
  public:
    void Init(Registry& registry) override;
    static void AttachToEntity(Registry& registry, Entity entity, float angle = 0.0f);
};

} // namespace Engine

#endif // ENGINE_PLUGINS_ROTATIONPLUGIN_HPP_
