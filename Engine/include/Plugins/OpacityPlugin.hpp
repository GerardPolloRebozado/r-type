#ifndef ENGINE_PLUGINS_OPACITYPLUGIN_HPP_
#define ENGINE_PLUGINS_OPACITYPLUGIN_HPP_

#include "Components/Opacity.hpp"
#include "Plugins/IPlugin.hpp"

namespace Engine {

class OpacityPlugin : public IPlugin {
  public:
    void Init(Registry& registry) override;
    static void AttachToEntity(Registry& registry, Entity entity, float alpha = 1.0f,
                               bool inheritToChildren = true);
    /**
     * @brief Computes the render alpha for an entity: its own Opacity times
     *        every ancestor's Opacity whose inheritToChildren flag is set.
     *        Returns 1.0 when the entity has no Opacity. Depth-capped so
     *        hand-built parent cycles terminate.
     */
    static float GetEffectiveAlpha(Registry& registry, Entity entity);
};

} // namespace Engine

#endif // ENGINE_PLUGINS_OPACITYPLUGIN_HPP_
