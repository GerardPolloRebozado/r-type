/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** MultiplayerSystem.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERSYSTEM_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERSYSTEM_HPP
#include "ECS/System.hpp"

namespace Engine {

class MultiplayerSystem : public System {
  public:
    ~MultiplayerSystem() noexcept override = default;

    /**
     * @brief It will poll for new packets and send the pending ones
     * @param registry registry of the game
     * @param dt time elapsed since last update
     */
    void Update(Registry& registry, float dt) override;
};
} // namespace Engine
#endif // G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERSYSTEM_HPP