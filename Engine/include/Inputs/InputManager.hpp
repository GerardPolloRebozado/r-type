/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** InputManager.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_INPUTMANAGER_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_INPUTMANAGER_HPP
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "IController.hpp"

namespace Engine {

class InputManager {
  public:
    InputManager();
    ~InputManager();
    void AddPlayer(int playerId, std::unique_ptr<IController> controller);
    void Update();
    IController* GetController(int playerId);

  private:
    std::unordered_map<int, std::unique_ptr<IController>> m_controllers;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_INPUTMANAGER_HPP
