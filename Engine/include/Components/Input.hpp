/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** Input.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_INPUT_HPP_
#define G_ING_500_BAR_5_1_RTYPE_1_INPUT_HPP_

namespace Engine {
struct Input {
    int playerIdx;
    bool isActive;

    Input(int playerIndex = 1, bool active = true) : playerIdx(playerIndex), isActive(active) {}
};
} // namespace Engine
#endif // G_ING_500_BAR_5_1_RTYPE_1_INPUT_HPP_
