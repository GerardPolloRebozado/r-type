/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** IController.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_ICONTROLLER_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_ICONTROLLER_HPP
#include "raylib-cpp.hpp"
#include <vector>

namespace Engine {
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
class IController {
  public:
    using Kmap = std::map<int, ActionType>;

    virtual ~IController();
    virtual void poll() = 0;
    virtual void Update() = 0;
    virtual void setDefaultKeymap() = 0;
    virtual const std::vector<ActionType>& getActionQueue() = 0;

  protected:
    std::vector<ActionType> _actionQueue;
    Kmap m_keymap;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_ICONTROLLER_HPP
