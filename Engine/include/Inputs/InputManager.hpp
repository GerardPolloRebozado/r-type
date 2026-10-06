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
#include <vector>

#include "IController.hpp"

namespace Engine {

class InputManager {
  public:
    InputManager(int playerId);
    ~InputManager();
    void AddPlayer(int playerId, std::unique_ptr<IController> controller);
    std::unique_ptr<IController> GetController(int playerId);
    void Update() {
        for (const auto& control : m_controllers) {
        }
    }

  private:
    std::map<int, std::unique_ptr<IController>> m_controllers;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_INPUTMANAGER_HPP
