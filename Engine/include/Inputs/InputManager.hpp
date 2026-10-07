/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** InputManager.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_INPUTMANAGER_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_INPUTMANAGER_HPP
#include <memory>
#include <unordered_map>
#include <vector>

#include "IController.hpp"

namespace Engine {

/**
 * @class InputManager
 * @brief Central input management resource storing and updating player controllers.
 *
 * Registered as a global resource in the ECS Registry. Polled during @ref Application::PreUpdate
 * to refresh all registered player controllers.
 */
class InputManager {
  public:
    InputManager();
    ~InputManager();

    /**
     * @brief Associates a player ID with a specific controller device.
     * @param playerId Identifier for the player (e.g. 1, 2).
     * @param controller Unique pointer to an @ref IController instance.
     */
    void AddPlayer(int playerId, std::unique_ptr<IController> controller);

    /**
     * @brief Updates all registered controllers for the current frame.
     */
    void Update();

    /**
     * @brief Retrieves the controller assigned to a given player ID.
     * @param playerId Identifier for the player.
     * @return Pointer to @ref IController if found, nullptr otherwise.
     */
    IController* GetController(int playerId);

  private:
    std::unordered_map<int, std::unique_ptr<IController>> m_controllers;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_INPUTMANAGER_HPP
