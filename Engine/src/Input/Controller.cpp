/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** Controller.cpp
*/
#include "Inputs/Controller.hpp"
#include <algorithm>

namespace Engine {

Controller::Controller(int gamepadId) : m_gamepadId(gamepadId) {}

bool Controller::isKeybindPressed(const Keybind& kb) const {
    bool mainPressed = false;
    switch (kb.type) {
        case KEYBOARD:
            mainPressed = IsKeyDown(kb.key);
            break;
        case MOUSE:
            mainPressed = IsMouseButtonDown(kb.key);
            break;
        case GAMEPAD:
            mainPressed = IsGamepadButtonDown(m_gamepadId, kb.key);
            break;
        default:
            break;
    }

    if (!mainPressed) {
        return false;
    }

    if (kb.composeKey != 0) {
        return IsKeyDown(kb.composeKey);
    }
    return true;
}

void Controller::Update() {
    _activeActions.clear();
    for (const auto& [action, keybinds] : m_keymap) {
        for (const auto& kb : keybinds) {
            if (isKeybindPressed(kb)) {
                _activeActions.push_back(action);
                break;
            }
        }
    }
}

void Controller::addKeybind(const Action& action, const Keybind& kb) {
    m_keymap[action].push_back(kb);
}

bool Controller::isActionActive(const std::string& action) const {
    return std::find(_activeActions.begin(), _activeActions.end(), action) != _activeActions.end();
}

const std::vector<IController::Action>& Controller::getActiveActions() const {
    return _activeActions;
}

} // namespace Engine