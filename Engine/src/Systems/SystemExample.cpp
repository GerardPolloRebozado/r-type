#include "Systems/SystemExample.hpp"
#include "Components/ComponentExample.hpp"
#include "Components/PluginExampleEvents.hpp"

namespace Engine {
void SystemExample::Update(Registry& registry, float dt) {
    for (auto const& entity : m_Entities) {
        auto& data = registry.GetComponent<ComponentExample>(entity);

        // Example Logic: Increment data over time
        data.someData += dt;

        // When a certain condition is met...
        if (data.someData >= 1.0f) {
            data.someData = 0.0f;
            data.someState++;

            // If this entity has user-defined hooks, fire them safely
            if (registry.HasComponent<PluginExampleEvents>(entity)) {
                auto& events = registry.GetComponent<PluginExampleEvents>(entity);

                // Trigger all connected callbacks
                for (auto& hook : events.onStateChanged) {
                    hook(entity, data.someState);
                }
            }
        }
    }
}
} // namespace Engine
