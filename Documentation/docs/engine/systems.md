# Systems & Components

## Built-in Components
- `Transform`: Position and scale.
- `AudioSource`: Sound effect and music state.

## Creating a System
To create a system, inherit from `Engine::System` and override `Update(Registry& registry, float dt)`.

```cpp
class MovementSystem : public Engine::System {
public:
    void Update(Engine::Registry& registry, float dt) override {
        for (auto entity : m_Entities) {
            auto& transform = registry.GetComponent<Transform>(entity);
            transform.x += 10.0f * dt;
        }
    }
};
```
