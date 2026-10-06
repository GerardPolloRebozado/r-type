/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** Controller.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_CONTROLLER_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_CONTROLLER_HPP
#include "IController.hpp"

namespace Engine {
/**
 * @class Controller
 * @brief Controller implementation polling Raylib keyboard, mouse, and gamepad inputs.
 */
class Controller : public IController {
  public:
    /**
     * @brief Constructs a Controller.
     * @param gamepadId Gamepad index (default 0).
     */
    Controller(int gamepadId = 0);

    ~Controller() override = default;

    /**
     * @brief Evaluates all registered keybinds and updates the list of active actions.
     */
    void Update() override;

    /**
     * @brief Binds a physical button/key to an action name.
     */
    void addKeybind(const Action& action, const Keybind& kb) override;

    /**
     * @brief Checks if a specific action is active this frame.
     */
    bool isActionActive(const std::string& action) const override;

    /**
     * @brief Retrieves all active actions for this frame.
     */
    const std::vector<Action>& getActiveActions() const override;

    /**
     * @brief Sets the gamepad ID for this controller.
     */
    void setGamepadId(int gamepadId) {
        m_gamepadId = gamepadId;
    }

  private:
    int m_gamepadId;

    bool isKeybindPressed(const Keybind& kb) const;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_CONTROLLER_HPP
