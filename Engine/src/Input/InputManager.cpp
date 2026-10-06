/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** InputManager.cpp
*/
#include "../../include/Inputs/InputManager.hpp"

#include "Keyboard.hpp"

namespace Engine {
InputManager::InputManager(int playerId) {}

InputManager::~InputManager() {}

void InputManager::AddPlayer(int playerId, std::unique_ptr<IController> controller) {
    m_controllers.insert(playerId, controller);
}

std::unique_ptr<IController> InputManager::GetController(int playerId) {
    return std::move(m_controllers.at(playerId));
}
} // namespace Engine