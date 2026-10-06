/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** IController.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_ICONTROLLER_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_ICONTROLLER_HPP
#include "raylib-cpp.hpp"
#include <unordered_map>
#include <vector>

namespace Engine {

/**
 * @enum ActionType
 * @brief Abstract game actions independent of physical input devices.
 */

/**
 * @class IController
 * @brief Abstract interface representing any input device controller (keyboard, gamepad, network).
 *
 * Translates hardware or remote input into abstract @ref ActionType values stored in an action
 * queue.
 */
enum DeviceType { KEYBOARD, MOUSE, GAMEPAD, TOUCH };
struct Keybind {
    int key;
    int composeKey;
    DeviceType type;
};
class IController {
  public:
    using Action = std::string;
    using Kmap = std::unordered_map<Action, std::vector<Keybind>>;
    virtual ~IController() = default;
    /**
     * @brief Updates the active action queue based on current hardware input.
     */
    virtual void Update() = 0;

    virtual void addKeybind(Action action, Keybind kb) = 0;
    virtual bool isActionActive(const std::string& action) const = 0;
    virtual const std::vector<std::string>& getActiveActions() const = 0;

    /**
     * @brief Returns the list of actions triggered during the current frame.
     * @return Const reference to the active action queue.
     */
    virtual const std::vector<std::pair<DeviceType, Action>>& getActionQueue() = 0;

  protected:
    /** @brief Active actions triggered during this tick/frame. */
    std::vector<std::pair<DeviceType, Action>> _actionQueue;

    /** @brief Mapping of physical key codes to abstract actions. */
    Kmap m_keymap;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_ICONTROLLER_HPP
