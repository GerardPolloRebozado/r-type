#ifndef G_ING_500_BAR_5_1_RTYPE_1_STATESSYSTEM_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_STATESSYSTEM_HPP
#include "ECS/Registry.hpp"
#include "ECS/System.hpp"

namespace Engine {
    template<typename T>
    class StatesSystem : public System {
    public:
        void Update(Registry &registry, float dt) override {
            auto res = registry.GetResource<StateResource<T> >();
            if (res.m_nextValue.has_value()) {
                res.onChanged(registry, res.m_value, res.m_nextValue);
                res.m_value = res.m_nextValue;
            }
        }
    };
}
#endif //G_ING_500_BAR_5_1_RTYPE_1_STATESSYSTEM_HPP
