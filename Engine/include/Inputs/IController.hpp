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
typedef enum ActionType {
    NO_ACTION = 0,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_LEFT,
    ACTION_RIGHT,
    ACTION_1,
    ACTION_2,
    ACTION_3,
    ACTION_4,
    MAX_ACTION
} ActionType;

/**
 * @class IController
 * @brief Abstract interface representing any input device controller (keyboard, gamepad, network).
 *
 * Translates hardware or remote input into abstract @ref ActionType values stored in an action
 * queue.
 */
class IController {
  public:
    using Kmap = std::unordered_map<int, ActionType>;

    virtual ~IController() = default;

    /**
     * @brief Polls device input state.
     */
    virtual void poll() = 0;

    /**
     * @brief Updates the active action queue based on current hardware input.
     */
    virtual void Update() = 0;

    /**
     * @brief Sets default key/button bindings for this controller.
     */
    virtual void setDefaultKeymap() = 0;

    /**
     * @brief Returns the list of actions triggered during the current frame.
     * @return Const reference to the active action queue.
     */
    virtual const std::vector<ActionType>& getActionQueue() = 0;

  protected:
    /** @brief Active actions triggered during this tick/frame. */
    std::vector<ActionType> _actionQueue;

    /** @brief Mapping of physical key codes to abstract actions. */
    Kmap m_keymap;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_ICONTROLLER_HPP
