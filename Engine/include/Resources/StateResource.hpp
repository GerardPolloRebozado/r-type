#ifndef G_ING_500_BAR_5_1_RTYPE_1_STATECOMPONENT_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_STATECOMPONENT_HPP

#include <optional>
#include <vector>
#include <functional>

namespace Engine {
    class Registry;

    template<typename T>
    struct StateResource {
        std::optional<T> m_nextValue;
        T m_value;

        std::vector<std::function<void(Registry &, T previousValue, T nextValue)> > onChanged;

        explicit StateResource(T initial) : m_nextValue(std::nullopt), m_value(initial) {
        }

        void SetValue(T value) {
            m_nextValue = value;
        }
    };
} // namespace Engine
#endif // G_ING_500_BAR_5_1_RTYPE_1_STATECOMPONENT_HPP
