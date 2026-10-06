/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** InputManager.cpp
*/
#include "../../include/Inputs/InputManager.hpp"

#include "Keyboard.hpp"

namespace Engine {
InputManager::InputManager() {}

InputManager::~InputManager() = default;

void InputManager::AddPlayer(int playerId, std::unique_ptr<IController> controller) {
    m_controllers[playerId] = std::move(controller);
}

void InputManager::Update() {
    for (const auto& control : m_controllers) {
        control.second->Update();
    }
}

IController* InputManager::GetController(int playerId) {
    auto it = m_controllers.find(playerId);
    if (it != m_controllers.end()) {
        return it->second.get();
    }
    return nullptr;
}
} // namespace Engine