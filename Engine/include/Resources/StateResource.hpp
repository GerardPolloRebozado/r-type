#ifndef G_ING_500_BAR_5_1_RTYPE_1_STATECOMPONENT_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_STATECOMPONENT_HPP

#include <optional>

/**
 * @file StateResource.hpp
 * @brief State machine resources used by the state system.
 */
namespace Engine {

/**
 * @class State
 * @brief Read-only registry resource holding the current state value.
 * @tparam T Enum or POD type describing the game state.
 */
template <typename T> struct State {
    /** @brief Currently active state value. */
    T value;

    explicit State(T initial) : value(initial) {}
};

/**
 * @class NextState
 * @brief Registry resource used to request a transition to another state.
 * @tparam T Enum or POD type describing the game state.
 */
template <typename T> struct NextState {
    /** @brief Pending state to enter; nullopt when no transition was requested. */
    std::optional<T> value = std::nullopt;

    /**
     * @brief Request a state transition.
     * @param next State that should become active.
     */
    void set(T next) {
        value = next;
    }
};

/**
 * @class StateTransitionEvent
 * @brief Event emitted when the state system changes the active state.
 * @tparam T Enum or POD type describing the game state.
 */
template <typename T> struct StateTransitionEvent {
    /** @brief State left during this transition, when any. */
    std::optional<T> exited = std::nullopt;

    /** @brief State entered during this transition, when any. */
    std::optional<T> entered = std::nullopt;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_STATECOMPONENT_HPP