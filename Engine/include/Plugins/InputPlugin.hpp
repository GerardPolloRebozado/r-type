/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** InputPlugin.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_INPUTPLUGIN_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_INPUTPLUGIN_HPP
#include "IPlugin.hpp"

namespace Engine {
/**
 * @class InputPlugin
 * @brief Bundles the @ref Input component and @ref InputManager resource into the ECS registry.
 *
 * Provides initialization logic and utility methods to attach the Input component to entities.
 */
class InputPlugin : public IPlugin {
  public:
    /**
     * @brief Registers the @ref Input component and ensures the @ref InputManager resource is
     * registered.
     * @param registry Main ECS registry.
     */
    void Init(Registry& registry) override;

    /**
     * @brief Attaches an @ref Input component to a specified entity.
     * @param registry Main ECS registry.
     * @param entity Target entity to attach input control to.
     * @param idxPlayer Player identifier.
     */
    static void AttachToEntity(Registry& registry, Entity entity, int idxPlayer);
};
} // namespace Engine
#endif // G_ING_500_BAR_5_1_RTYPE_1_INPUTPLUGIN_HPP
