# Resources

Resources are global, one-per-game values shared between systems: asset tables, input state,
the network host, project paths, scene management. They are not attached to entities.

## Adding and reading

```cpp
auto& assets = registry.AddResource<Engine::AssetManager>();
auto& input  = registry.GetResource<Engine::InputManager>(); // throws if missing
bool has     = registry.HasResource<Engine::InputManager>();
```

`AddResource<T>(args...)` constructs in place and returns a reference. `GetResource<T>()`
throws `ErrorEcs` if the resource is missing. Guard shared resources with `HasResource<T>()`
inside plugins so two plugins do not add the same one twice.

## Built-in resources

| Resource | Provided by | Purpose |
|---|---|---|
| `AssetManager` | `SpritePlugin` / `AudioPlugin` | Loaded sprites, sounds, music, fonts |
| `InputManager` | `InputPlugin` | Controller instances per player |
| `VirtualResolution` | `Application` | Virtual canvas and letterboxing |
| `LaunchOptions` | `Application` | Command-line arguments |
| `ProjectContext` | Editor / scene loading | Project folder and path resolution |
| `SceneManager` | Scene loading | Pending scene switch requests |
| `State<T>` / `NextState<T>` / `StateTransitionEvent<T>` | User | State machine state |
| `NetworkHostResource` | `MultiplayerPlugin` | ENet host and polling |
| `PacketRegistryResource` | `MultiplayerPlugin` | Deterministic packet name-to-id map |
| `PacketRouterResource` | `MultiplayerPlugin` | Packet id-to-handler dispatch |
| `PlayerInputState` | `MultiplayerPlugin` | Server-side network id to last input |
| `PlayerSlotState` | `MultiplayerPlugin` | Network id to color slot (max 5) |
| `PrefabSpawner` | `Serialization::SceneSerializer` | Runtime prefab instantiation hook |
| `EditorMode` | Editor | Tag resource that keeps `ScriptSystem` idle in the editor |

## Component vs. resource

Use a **component** for anything that is one per entity (position, health, sprite). Use a
**resource** for anything that is one per game (asset table, input state, network host).
A system can read both in the same frame.
