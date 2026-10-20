# Plugins

A **plugin** is a bundle: it registers the components, systems, and resources a feature
needs, in the right order, behind one `Init()` call. Plugins keep `main()` readable and make
features removable.

If the registry is the engine's parts bin, a plugin is the subassembly.

---

## Why plugins

Without one, wiring audio looks like this:

```cpp
registry.AddResource<AssetManager>();
registry.RegisterComponent<AudioSource>();
registry.RegisterSystem<AudioSystem>();
// ...and remember the update order relative to every other system
```

With one:

```cpp
Engine::AudioPlugin audioPlugin;
audioPlugin.Init(app.GetRegistry());
```

The plugin owns the details. Game code owns the decision to include audio at all.

---

## The `IPlugin` interface

```cpp
#include "Plugins/IPlugin.hpp"

namespace Engine {
class IPlugin {
  public:
    virtual ~IPlugin() = default;
    virtual void Init(Registry& registry) = 0;
};
} // namespace Engine
```

That is the whole contract. `Init()` runs once, during startup, before the first frame.

---

## Writing a plugin

```cpp
#include "Plugins/IPlugin.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/AudioSource.hpp"
#include "Systems/AudioSystem.hpp"

namespace Engine {
class AudioPlugin : public IPlugin {
  public:
    void Init(Registry& registry) override {
        // 1. Resources first: systems may read them on their first update.
        if (!registry.HasResource<AssetManager>()) {
            registry.AddResource<AssetManager>();
        }

        // 2. Components: systems look them up by type.
        registry.RegisterComponent<AudioSource>();

        // 3. Systems: registration order is update order.
        registry.RegisterSystem<AudioSystem>();
    }
};
} // namespace Engine
```

Order matters:

1. **Resources** — anything a system assumes exists.
2. **Components** — the data the system reads.
3. **Systems** — the behaviour, in dependency order.

If two plugins share a resource (many do: `AssetManager`), guard with `HasResource<T>()`
instead of adding it twice.

---

## Using a plugin in `main()`

```cpp
Engine::Application app(config);

Engine::HierarchyPlugin hierarchy;
hierarchy.Init(app.GetRegistry());

Engine::PositionPlugin position;
position.Init(app.GetRegistry());

Engine::SpritePlugin sprites;
sprites.Init(app.GetRegistry());

app.Run();
```

Register hierarchy before position: hierarchy computes parent motion, position snapshots the
result. Reversing them makes children drift by one frame.

---

## Attaching data

Some plugins expose static helpers so callers do not have to know the component layout:

```cpp
Engine::SpritePlugin::AttachToEntity(
    registry, entity,
    Engine::SpriteComponent{ .assetPath = "assets/player.png" });

Engine::TextPlugin::AttachToEntity(registry, entity, "SCORE 0", 24.0f);
```

The helper adds the component, plus a default `Position` if the entity has none.

---

## Built-in plugins

| Plugin | Registers |
|---|---|
| `PositionPlugin` | `Position`, `PositionSystem` |
| `RotationPlugin` | `Rotation`, `RotationSystem` |
| `HierarchyPlugin` | `Parent`, `HierarchySystem` |
| `SpritePlugin` | `SpriteComponent`, `SpriteSystem`, `AssetManager` |
| `ShapePlugin` | `ShapeComponent`, `ShapeSystem` |
| `TextPlugin` | `Text`, `TextSystem` |
| `CameraPlugin` | `CameraComponent`, `CameraSystem` |
| `ColliderPlugin` | `ColliderComponent`, `CollisionSystem` |
| `InputPlugin` | `Input`, `InputManager`, input systems |
| `AudioPlugin` | `AudioSource`, `AudioSystem`, `AssetManager` |
| `OpacityPlugin` | `Opacity`, opacity system |
| `SignalPlugin` | `SignalConnections`, signal dispatch |
| `ScriptPlugin` | `ScriptComponent`, `ScriptSystem` |
| `ReplicationPlugin` | replication resources and system |
| `MultiplayerPlugin` | network host, packet router, multiplayer system |

The exact ordering used by the real game is in `Games/R-Type/main.cpp`; it is a good
reference when your plugin depends on another.

---

## Scaffolding a plugin

The editor can generate the header, source, and CMake entries:

```bash
./RTypeEditor create plugin Health
```

It writes `HealthPlugin` (the suffix is added if missing) into the engine tree, or into
`system/` when run inside a project. See [Command line](../editor/cli.md).

---

## Rules of thumb

- A plugin registers; it does not own runtime state.
- Make `Init()` idempotent: guard resources and component registration.
- Keep one plugin per feature, not one per file.
- Document the ordering constraints next to the registration.

Next: [Asset management](assets.md) or back to [ECS architecture](ecs.md).
