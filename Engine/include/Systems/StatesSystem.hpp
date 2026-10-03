#ifndef G_ING_500_BAR_5_1_RTYPE_1_STATESSYSTEM_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_STATESSYSTEM_HPP

#include "ECS/Registry.hpp"
#include "ECS/System.hpp"
#include "Resources/StateResource.hpp"

namespace Engine {
    /**
     * @brief System responsible for applying state transitions.
     */
    template<typename T>
    class StatesSystem : public System {
    public:
        void Update(Registry &registry, float dt) override {
            if (!registry.HasResource<State<T> >() || !registry.HasResource<NextState<T> >()) {
                return;
            }

            auto &state = registry.GetResource<State<T> >();
            auto &nextState = registry.GetResource<NextState<T> >();

            if (nextState.value.has_value()) {
                T prev = state.value;
                T next = nextState.value.value();

                if (prev != next) {
                    state.value = next;

                    if (registry.HasResource<StateTransitionEvent<T> >()) {
                        auto &event = registry.GetResource<StateTransitionEvent<T> >();
                        event.exited = prev;
                        event.entered = next;
                    }
                }

                nextState.value = std::nullopt;
            }
        }
    };
}

#endif //G_ING_500_BAR_5_1_RTYPE_1_STATESSYSTEM_HPP
