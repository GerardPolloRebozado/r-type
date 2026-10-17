/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** MultiplayerPlugin.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERPLUGIN_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERPLUGIN_HPP
#include "IPlugin.hpp"

namespace Engine::Multiplayer {
/**
 * @class MultiplayerPlugin
 * @brief Interface for plugins.
 *
 * Plugins allow users to inject combinations of components, systems,
 * and functionality into the engine.
 */
class MultiplayerPlugin : public IPlugin {
  public:
    ~MultiplayerPlugin() override = default;

    MultiplayerPlugin() = default;

    /**
     * @brief Initializes the plugin, registering components and systems.
     * @param registry A reference to the main ECS registry.
     */
    void Init(Registry& registry) override;
};
} // namespace Engine::Multiplayer
#endif // G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERPLUGIN_HPP