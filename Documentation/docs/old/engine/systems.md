# Systems

A **system** is where behaviour lives. It reads and writes components, once per frame,
for every entity that has the data it cares about.

Components are passive ([Components](components.md)); systems are the only place where
values change.

---

## The `System` interface

Every system inherits from `Engine::System` and overrides the passes it needs:

```cpp
#include "ECS/System.hpp"

class MovementSystem : public Engine::System {
  public:
    void Update(Engine::Registry& registry, float dt) override {
        auto positions = registry.GetComponentArray<Engine::Position>();

        for (auto& [entity, position] : *positions) {
            position.x += 120.0f * dt;
        }
    }
};
```

The three passes run in this order every frame:

| Pass | Signature | Environment | Typical use |
|---|---|---|---|
| Update | `Update(Registry&, float dt)` | Before rendering | Simulate, poll input, resolve collisions |
| Render | `Render(Registry&)` | Inside the active camera | World-space drawing |
| RenderScreen | `RenderScreen(Registry&)` | Window pixels, no camera | HUD, menus, overlays |

`dt` is the frame delta in seconds. Multiply everything time-based by it.

!!! note "Entity membership is explicit"
    `System` exposes an `m_Entities` set, but the registry does **not** fill it
    automatically from a signature in this build. Built-in systems either iterate a
    component array (`GetComponentArray<T>()`) or have their set populated by their
    plugin. If you want the latter, keep it in sync when you add or remove the
    component. For most gameplay systems, iterating the component array is simpler and
    always correct.

---

## Registering a system

Systems are registered once, at startup:

```cpp
auto movement = registry.RegisterSystem<MovementSystem>();
```

Registration order is update order. If system B reads data written by system A, register A
first:

```cpp
registry.RegisterSystem<HierarchySystem>();   // first: propagate parent motion
registry.RegisterSystem<PositionSystem>();    // then: snapshot previous positions
```

`RegisterSystem<T>()` returns a `std::shared_ptr<T>` if you need to configure the system
afterwards:

```cpp
auto collision = registry.RegisterSystem<CollisionSystem>();
collision->SetLayerMask(/* ... */);
```

---

## Iterating entities

### Over one component array

```cpp
for (auto& [entity, health] : *registry.GetComponentArray<Health>()) {
    health.current -= 1;
}
```

`GetComponentArray<T>()` returns a `ComponentArray<T>` whose iterators yield
`std::pair<const Entity, T>`. It throws `ErrorEcs` if `T` was never registered.

### Over several components

There is no hidden query language: check each entity against the components you need.

```cpp
auto& positions = *registry.GetComponentArray<Position>();

for (auto& [entity, position] : positions) {
    if (!registry.HasComponent<Velocity>(entity)) {
        continue;
    }
    auto& velocity = registry.GetComponent<Velocity>(entity);
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
}
```

Keep the check cheap and the component types narrow. That is the ECS trade-off: a few
`HasComponent` calls in exchange for data laid out contiguously.

### Over every entity

Entity IDs are monotonically allocated and may have gaps. The editor uses
`GetEntityCount()` to scan the range:

```cpp
for (std::size_t i = 1; i < registry.GetEntityCount(); ++i) {
    Engine::Entity entity{static_cast<std::uint32_t>(i)};
    if (registry.HasComponent<Engine::Name>(entity)) {
        // ...
    }
}
```

Prefer a component array when one exists; only scan the ID range when you genuinely need
"all entities".

---

## Destroying entities safely

Never call `DestroyEntity()` while iterating. Queue the destruction and let the frame finish:

```cpp
if (health.current <= 0) {
    registry.QueueDestroyEntity(entity);
}
```

The queue is flushed at the end of the frame, after every system has run. Destroying a
parent also destroys its descendants (anything whose `Parent` points at it).

---

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
        // Draw at registry.GetComponent<Position>(entity)
    }
};
```

`screenSpace == true` is the HUD pass; return `false` there unless you are drawing a HUD
element.

---

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
| `StatesSystem` | Applies `NextState` transitions |
| `InputSystem` / `InputManager` | Polls devices into actions |
| `ScriptSystem` | Runs Lua scripts |
| `ReplicationSystem` / `MultiplayerSystem` | Snapshot replication and networking |
| `SignalSystem` | Dispatches typed signals |

Most of these are installed by their plugin, not by hand. See [Plugins](plugins.md).

---

## Rules of thumb

- One system, one job.
- Read `dt`; never assume 60 FPS.
- Do not own other entities from a system; use components to link them.
- Register in dependency order.
- Queue destruction; never mutate the container you are iterating.

Next: [Plugins](plugins.md).
