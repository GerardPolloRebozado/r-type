#ifndef G_ING_500_BAR_5_1_RTYPE_1_STATECOMPONENT_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_STATECOMPONENT_HPP

#include <optional>

namespace Engine {
// Read-only resource
template <typename T> struct State {
    T value;

    explicit State(T initial) : value(initial) {}
};

// Resource to write new states
template <typename T> struct NextState {
    std::optional<T> value = std::nullopt;

    void set(T next) {
        value = next;
    }
};

// Event that is send when a state changes and is triggered by the state system
template <typename T> struct StateTransitionEvent {
    std::optional<T> exited = std::nullopt;
    std::optional<T> entered = std::nullopt;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_STATECOMPONENT_HPP
