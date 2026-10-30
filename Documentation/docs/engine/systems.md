# Systems

A **system** is where behaviour lives. It reads and writes components, once per fixed step,
for every entity that has the data it cares about.

Components are passive ([Components](components.md)); systems are the only place where values
change.

## The `System` interface

Every system inherits from `Engine::System` and overrides the passes it needs:

```cpp
#include "ECS/System.hpp"

class MovementSystem : public Engine::System {
  public:
    void Update(Engine::Registry& registry, float dt) override {
        for (auto& [entity, position] : *registry.GetComponentArray<Engine::Position>()) {
            position.x += 120.0f * dt;
        }
    }
};
```

The three passes run in this order every frame:

| Pass | Signature | Environment | Typical use |
|---|---|---|---|
| `Update` | `Update(Registry&, float dt)` | Before rendering | Simulate, poll input, resolve collisions |
| `Render` | `Render(Registry&)` | Inside the active camera | World-space drawing |
| `RenderScreen` | `RenderScreen(Registry&)` | Window pixels, no camera | HUD, menus, overlays |

`dt` is the frame delta in seconds. Multiply everything time-based by it. `OnSceneUnloaded`
is called right before a scene's entities are destroyed, so systems can drop state kept
outside components.

## Registering a system

```cpp
auto movement = registry.RegisterSystem<MovementSystem>(); // shared_ptr<T>
```

Registration order is update order. If system B reads data written by system A, register A
first:

```cpp
registry.RegisterSystem<HierarchySystem>();   // first: propagate parent motion
registry.RegisterSystem<PositionSystem>();    // then: snapshot previous positions
```

`RegisterSystem<T>()` returns a `std::shared_ptr<T>` if you need to configure the system
afterwards.

## Iterating entities

Over one component array:

```cpp
for (auto& [entity, health] : *registry.GetComponentArray<Health>()) {
    health.current -= 1;
}
```

Over several components:

```cpp
auto& positions = *registry.GetComponentArray<Position>();

for (auto& [entity, position] : positions) {
    if (!registry.HasComponent<Velocity>(entity)) {
        continue;
    }
    auto& velocity = registry.GetComponent<Velocity>(entity).value().get();
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
}
```

There is no hidden query language. The trade-off is a few `HasComponent` calls in exchange
for data laid out contiguously.

## Destroying entities safely

Never call `DestroyEntity()` while iterating. Queue the destruction and let the frame
finish:

```cpp
if (health.current <= 0) {
    registry.QueueDestroyEntity(entity);
}
```

The queue is flushed at the end of the frame, after every system has run. Destroying a
parent also destroys its descendants.

## Rendering systems

World-space renderers usually derive from `LayeredRenderSystem` instead of overriding
`Render` directly. The renderer calls `CanDraw()` for every entity and then draws the
matches sorted by `Position::z`, so sprites, shapes, and text can interleave correctly.

```cpp
class HealthBarSystem : public Engine::LayeredRenderSystem {
  public:
    bool CanDraw(Engine::Registry& registry, Engine::Entity entity,
                 bool screenSpace) override {
        return !screenSpace && registry.HasComponent<Health>(entity);
    }

    void DrawEntity(Engine::Registry& registry, Engine::Entity entity) override {
        // Draw at registry.RequireComponent<Position>(entity)
    }
};
```

`screenSpace == true` is the HUD pass; return `false` there unless you are drawing a HUD
element.

## Built-in systems

| System | Responsibility |
|---|---|
| `HierarchySystem` | Propagates parent motion to children |
| `PositionSystem` | Snapshots previous positions and computes `moved` |
| `RotationSystem` | Applies rotation |
| `ShapeSystem` | Draws primitive shapes |
| `SpriteSystem` | Draws `SpriteComponent` |
| `TextSystem` | Draws `Text` |
| `CameraSystem` | Follow, bounds, zoom, and shake |
| `CollisionSystem` | SAT collision detection and events |
| `AudioSystem` | Consumes `AudioSource` requests |
| `StatesSystem<T>` | Applies `NextState` transitions |
| `InputManager` | Polls devices into actions |
| `ScriptSystem` | Runs Lua scripts |
| `ReplicationSystem` / `MultiplayerSystem` | Snapshot replication and networking |
| `SignalSystem` | Dispatches typed signals |

Most of these are installed by their plugin, not by hand. See [Plugins](../plugins/engine-plugins.md).
