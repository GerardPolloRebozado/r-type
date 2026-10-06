/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** KeyboardController.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_KEYBOARDCONTROLLER_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_KEYBOARDCONTROLLER_HPP
#include "IController.hpp"

namespace Engine {
/**
 * @class KeyboardController
 * @brief Concrete controller implementation using Raylib keyboard polling.
 */
class KeyboardController : public IController {
  public:
    /**
     * @brief Constructs a KeyboardController and initializes default key bindings.
     */
    KeyboardController();

    ~KeyboardController() override;

    /**
     * @brief Polls keyboard input (delegates to Update).
     */
    void poll() override;

    /**
     * @brief Configures standard WASD and action key mappings.
     */
    void setDefaultKeymap() override;

    /**
     * @brief Clears the action queue and polls Raylib for pressed keys.
     */
    void Update() override;

    /**
     * @brief Retrieves the active action queue for this frame.
     * @return Const reference to active action vector.
     */
    const std::vector<ActionType>& getActionQueue() override;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_KEYBOARDCONTROLLER_HPP
