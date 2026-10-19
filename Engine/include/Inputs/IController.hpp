/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** IController.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_ICONTROLLER_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_ICONTROLLER_HPP
#include "raylib-cpp.hpp"
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine {
using Action = std::string;
/**
 * @enum DeviceType
 * @brief Input device categories.
 */
enum DeviceType { KEYBOARD, MOUSE, GAMEPAD, TOUCH };

/**
 * @struct Keybind
 * @brief Binding descriptor mapping a physical button/key to an action.
 */
struct Keybind {
    int key = 0;
    int composeKey = 0;
    DeviceType type = KEYBOARD;
};

/**
 * @struct ActionBinding
 * @brief All shortcuts bound to one action name, plus whether that action is sent to the server.
 */
struct ActionBinding {
    std::vector<Keybind> keybinds;

    /** @brief When true this action is sent to the server in player_input. */
    bool networked = false;
};

/**
 * @class IController
 * @brief Abstract interface representing a player's input controller (keyboard, mouse, gamepad,
 * network).
 *
 * Translates hardware or remote input into dynamic, string-based game actions.
 */
class IController {
  public:
    using Kmap = std::unordered_map<Action, ActionBinding>;

    virtual ~IController() = default;

    /**
     * @brief Updates the active action list for this frame based on hardware input.
     */
    virtual void Update() = 0;

    /**
     * @brief Binds a key/button to an abstract action name.
     * @param action The string identifier of the action (e.g. "JUMP", "SHOOT").
     * @param kb The Keybind descriptor.
     * @param networked When true, marks the action as sent to the server in player_input.
     */
    virtual void addKeybind(const Action& action, const Keybind& kb, bool networked = false) = 0;

    /**
     * @brief Checks if an action is currently active in this frame.
     * @param action The action name to check.
     * @return True if active, false otherwise.
     */
    virtual bool isActionActive(const std::string& action) const = 0;

    /**
     * @brief Returns all actions currently active in this frame that should be send to the server
     * @return Const reference to active actions list that should be sent to the server
     */
    virtual const std::set<Action>& getNetworkedActiveActions() const = 0;

    /**
     * @brief Returns all actions currently active in this frame.
     * @return Const reference to active actions list.
     */
    virtual const std::set<Action>& getActiveActions() const = 0;

    /** @brief Active actions triggered during this tick/frame. */
    std::set<Action> _activeActions;

    /** @brief Subset of _activeActions whose action is marked networked. */
    std::set<Action> _networkedActions;

    /** @brief Mapping of action names to their shortcuts and network flag. */
    Kmap m_keymap;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_ICONTROLLER_HPP
