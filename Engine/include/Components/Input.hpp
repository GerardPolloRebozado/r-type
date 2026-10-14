/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** Input.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_INPUT_HPP_
#define G_ING_500_BAR_5_1_RTYPE_1_INPUT_HPP_

namespace Engine {
/**
 * @struct Input
 * @brief Component linking an entity to a specific player's controller.
 *
 * Used by gameplay systems to identify which player controls this entity
 * and whether the entity is currently active to receive commands.
 */
struct Input {
    /** @brief Player identifier corresponding to an attached controller in InputManager. */
    int playerIdx = 1;

    /** @brief Whether this entity is currently receiving and processing input. */
    bool isActive = true;

    /**
     * @brief Creates an Input component.
     * @param playerIndex The player ID (default: 1).
     * @param active Whether the input is active (default: true).
     */
    static Input Create(int playerIndex = 1, bool active = true) {
        Input input;
        input.playerIdx = playerIndex;
        input.isActive = active;
        return input;
    }
};
} // namespace Engine
#endif // G_ING_500_BAR_5_1_RTYPE_1_INPUT_HPP_
