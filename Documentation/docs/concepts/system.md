# Systems

In an ECS architecture, **Systems** hold all the game logic.

As described in the [Component](./component.md) page, components are simply passive data. To use and modify that data, we use the **Systems**. A system looks at the [Registry](./registry.md), finds every single entity (see [Registry](./registry.md)) that has a specific combination of components, and updates them.

## How it works

Systems do not run on a per-entity basis (like an object's `Update()` function). Instead, a System runs *once* per frame, and internally loops over all relevant entities. This is what (in theory) makes ECS so incredibly fast.

When you register a system, the `Registry` automatically maintains a list (`m_Entities`) of every entity that possesses the required components.

## Creating a System

1. Inherit from `Engine::System`.
2. Override the `Update(Registry&, float)` function.
3. Iterate over `m_Entities` and manipulate the components.

```cpp
#include "ECS/System.hpp"

class DamageSystem : public Engine::System {
public:
    void Update(Engine::Registry& registry, float dt) override {
        // m_Entities is automatically populated by the Registry!
        for (auto entity : m_Entities) {
            auto& health = registry.GetComponent<Health>(entity);

            if (health.current <= 0) {
                // Perform death logic or queue for destruction
                registry.QueueDestroyEntity(entity);
            }
        }
    }
};
```
