# Engine plugins

An engine plugin registers components, systems, and resources behind one `Init()` call. The
built-in plugins:

| Plugin | Registers |
|---|---|
| `PositionPlugin` | `Position`, `PositionSystem` |
| `RotationPlugin` | `Rotation` |
| `HierarchyPlugin` | `Parent`, `HierarchySystem` |
| `SpritePlugin` | `SpriteComponent`, `SpriteSystem`, `AssetManager` |
| `ShapePlugin` | `ShapeComponent`, `ShapeSystem`, plus `OpacityPlugin` |
| `TextPlugin` | `Text`, `TextSystem` |
| `CameraPlugin` | `CameraComponent`, `CameraSystem` |
| `ColliderPlugin` | `ColliderComponent`, `CollisionEvents`, `CollisionSystem` |
| `InputPlugin` | `Input`, `InputManager` |
| `AudioPlugin` | `AudioSource`, `AudioSystem`, `AssetManager` |
| `OpacityPlugin` | `Opacity` |
| `SignalPlugin` | `SignalConnections` |
| `ScriptPlugin` | `ScriptComponent`, `ScriptSystem` |
| `ReplicationPlugin` | `NetworkComponent`, `ReplicationSystem` |
| `MultiplayerPlugin` | `PlayerInformation`, network resources, `MultiplayerSystem` |

`ReplicationPlugin` must register before `MultiplayerPlugin`. See
[Networking](../engine/networking.md).

## Writing your own

```cpp
class AudioPlugin : public Engine::IPlugin {
  public:
    void Init(Engine::Registry& registry) override {
        if (!registry.HasResource<Engine::AssetManager>()) {
            registry.AddResource<Engine::AssetManager>();
        }
        registry.RegisterComponent<Engine::AudioSource>();
        registry.RegisterSystem<Engine::AudioSystem>();
    }
};
```

Order inside `Init()`: resources first, then components, then systems. Guard shared
resources with `HasResource<T>()`.

## Attaching data

Some plugins expose static helpers (`AttachToEntity`) so callers do not have to know the
component layout. See [Plugins (engine)](../plugins/engine-plugins.md).

## Scaffolding

```bash
./RTypeEditor create plugin Health
```

Writes the header, source, and CMake entries. See [Command line](../editor/cli.md).
