#ifndef ENGINE_PLUGINS_SIGNALPLUGIN_HPP_
#define ENGINE_PLUGINS_SIGNALPLUGIN_HPP_

#include "Components/SignalConnections.hpp"
#include "Plugins/IPlugin.hpp"

namespace Engine {

/**
 * @class SignalPlugin
 * @brief Registers the @ref SignalConnections component for Godot-style
 *        signal connections (`Component.connect` in Lua, Connections section
 *        in the editor Inspector).
 */
class SignalPlugin : public IPlugin {
  public:
    void Init(Registry& registry) override;

    /**
     * @brief Adds an empty @ref SignalConnections to an entity.
     */
    static void AttachToEntity(Registry& registry, Entity entity);
};

} // namespace Engine

#endif // ENGINE_PLUGINS_SIGNALPLUGIN_HPP_
