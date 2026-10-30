#ifndef ENGINE_PLUGINS_ANIMATIONPLUGIN_HPP_
#define ENGINE_PLUGINS_ANIMATIONPLUGIN_HPP_

#include "Components/Animation.hpp"
#include "Components/Position.hpp"
#include "Plugins/IPlugin.hpp"
#include "Systems/AnimationSystem.hpp"

namespace Engine {

class AnimationPlugin : public IPlugin {
  public:
    void Init(Registry& registry) override;

    static void AttachToEntity(Registry& registry, Entity entity, const Animation& animation);
};

} // namespace Engine

#endif // ENGINE_PLUGINS_ANIMATIONPLUGIN_HPP_
