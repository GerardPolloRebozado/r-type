#ifndef ENGINE_PLUGINS_REPLICATIONPLUGIN_HPP_
#define ENGINE_PLUGINS_REPLICATIONPLUGIN_HPP_

#include "Plugins/IPlugin.hpp"

namespace Engine::Multiplayer {

/**
 * @class ReplicationPlugin
 * @brief Registers @ref NetworkComponent and @ref ReplicationSystem so
 * entities can declare how they replicate across the network.
 */
class ReplicationPlugin : public IPlugin {
  public:
    void Init(Registry& registry) override;
};

} // namespace Engine::Multiplayer

#endif // ENGINE_PLUGINS_REPLICATIONPLUGIN_HPP_
