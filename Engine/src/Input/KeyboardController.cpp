/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** KeyboardController.cpp
*/
#include "../../include/Inputs/KeyboardController.hpp"

namespace Engine {
KeyboardController::KeyboardController() {
    setDefaultKeymap();
}

KeyboardController::~KeyboardController() {}

void KeyboardController::Update() {
    _actionQueue.clear();
    for (const auto& [key, action] : m_keymap) {
        if (IsKeyDown(key)) {
            _actionQueue.push_back(action);
        }
    }
}

const std::vector<ActionType>& KeyboardController::getActionQueue() {
    return _actionQueue;
}

void KeyboardController::poll() {
    Update();
}

void KeyboardController::setDefaultKeymap() {
    m_keymap = {{KEY_W, ACTION_UP},    {KEY_S, ACTION_DOWN},  {KEY_A, ACTION_LEFT},
                {KEY_D, ACTION_RIGHT}, {KEY_SPACE, ACTION_1}, {KEY_LEFT_SHIFT, ACTION_2},
                {KEY_E, ACTION_3},     {KEY_Q, ACTION_4}};
}
} // namespace Engine