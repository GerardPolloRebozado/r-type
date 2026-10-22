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
    _networkedActions.clear();
    for (const auto& [action, binding] : m_keymap) {
        for (const auto& kb : binding.keybinds) {
            if (!isKeybindPressed(kb)) {
                continue;
            }
            _activeActions.emplace(action);
            if (binding.networked) {
                _networkedActions.emplace(action);
            }
            break;
        }
    }
}

void Controller::addKeybind(const Action& action, const Keybind& kb, const bool networked) {
    ActionBinding& binding = m_keymap[action];
    binding.keybinds.push_back(kb);
    binding.networked = binding.networked || networked;
}

bool Controller::isActionActive(const std::string& action) const {
    return std::ranges::find(_activeActions, action) != _activeActions.end();
}

const std::set<Action>& Controller::getActiveActions() const {
    return _activeActions;
}

const std::set<Action>& Controller::getNetworkedActiveActions() const {
    return _networkedActions;
}
} // namespace Engine
