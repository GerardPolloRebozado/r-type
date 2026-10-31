#ifndef ENGINE_PLUGINS_WAVEPLUGIN_HPP_
#define ENGINE_PLUGINS_WAVEPLUGIN_HPP_

#include "Plugins/IPlugin.hpp"
#include "Systems/WaveSpawnerSystem.hpp"

namespace Engine {

/**
 * @class WavePlugin
 * @brief Plugin that bundles the scene wave spawner system.
 *
 * The wave data itself lives in the @ref WaveSchedule registry resource,
 * loaded from the scene config by the Serialization module.
 */
class WavePlugin : public IPlugin {
  public:
    void Init(Registry& registry) override;
};

} // namespace Engine

#endif // ENGINE_PLUGINS_WAVEPLUGIN_HPP_
