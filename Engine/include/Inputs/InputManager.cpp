/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** InputManager.cpp
*/
#include "InputManager.hpp"

#include "Keyboard.hpp"

namespace Engine {
InputManager::InputManager(int playerId) {}

InputManager::~InputManager() {
    raylib::Keyboard::IsKeyDown(KEY_ESCAPE);
    IsGamepadButtonDown();
}
} // namespace Engine