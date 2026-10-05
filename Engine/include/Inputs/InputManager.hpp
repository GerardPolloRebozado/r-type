/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** InputManager.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_INPUTMANAGER_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_INPUTMANAGER_HPP
#include <map>
#include <vector>

#include "IController.hpp"

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

class InputManager {
    using Kmap = std::map<ActionType, int>;

  public:
    InputManager(int playerId);
    ~InputManager();
    // used to load all keymaps saved before
    InitKeyMaps();

    ConfigKeyMap(int playerId, ActionType action, int key);

  private:
    Kmap _actionKeyMap;
    std::map<IController*, Kmap> _KeymapsMap;
    std::vector<int> _playerIds;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_INPUTMANAGER_HPP
