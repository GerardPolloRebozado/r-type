# Components

A **component** is a plain data struct attached to an entity. It has no logic and no
`Update()`. Systems read and write it.

```cpp
struct Health {
    int current = 100;
    int max = 100;
    bool invincible = false;
};
```

That is a complete, valid component.

## Design rules

1. **Data only.** No getters, setters, or update methods. Behaviour belongs in a system.
2. **Small and focused.** Prefer `Position` + `Velocity` over one `Transform` that also
   owns scale, tint, and physics. Systems subscribe to exactly what they read.
3. **No ownership.** Avoid `std::shared_ptr`, raw owning pointers, and heavy containers.
   Components are copied, moved, and serialized; keep them cheap and value-like.
4. **Serializable by default.** The editor reflects over public fields. Use types the
   reflection layer understands: `float`, `int`, `bool`, `std::string`, enums, and nested
   value structs.
5. **Stable field names.** They are the keys in `.scene` and `.prefab` files.

## Built-in components

| Component | Purpose |
|---|---|
| `Position` | World `x`, `y`, draw layer `z`, previous-frame values, `moved` flag, `relative` (follow parent delta) |
| `Rotation` | Angle in degrees |
| `ScreenSpace` | Tag: draw in window pixels, ignoring the camera (HUD) |
| `Parent` | Hierarchy link to another entity |
| `Name` | Human-readable label used by the editor and prefabs |
| `Opacity` | Alpha multiplier, with optional inheritance from ancestors |
| `SpriteComponent` | Sprite asset path, offsets, scale, flip |
| `ShapeComponent` | Rectangle, circle, regular polygon, or line |
| `Text` | String, font, size, spacing, colour |
| `CameraComponent` | Zoom, follow target, bounds, shake |
| `ColliderComponent` | Collision shape and layer/mask |
| `CollisionEvents` | Callbacks for collision and trigger lifecycle |
| `AudioSource` | Sound or music request |
| `Input` | Per-entity action state and player index |
| `ScriptComponent` | Lua script path and runtime state |
| `PrefabInstance` | Link back to the prefab that spawned this entity |
| `SignalConnections` | Subscriptions to typed signals |
| `NetworkComponent` | Replication flags and network identity |
| `PluginExampleEvents` | Example event component from the plugin sample |

Most of these have dedicated pages: [Rendering](rendering.md), [Collisions](collisions.md),
[Audio](audio.md), [Input](input.md), [Networking](networking.md), [Scripting](scripting.md).

## Tags are components too

A component with no fields is a **tag**. It carries one bit of information: "this entity is
different".

```cpp
struct PlayerTag {};
registry.AddComponent(player, PlayerTag{});
```

`ScreenSpace` is the built-in example: the renderer checks for it and switches to the HUD
pass. Use tags instead of booleans on unrelated components; they compose better.

## Reflection

Components opt into serialization through `Engine/include/Reflection/ComponentReflect.hpp`.
If a component is missing there, the editor cannot save it. See [Scene format](../editor/scenes.md).

## Notes on `Transform`

The engine does not have a `Transform` component. It uses separate `Position` and `Rotation`
components, plus `Parent` for hierarchy. When porting snippets, replace
`Transform{ .x, .y }` with `Position(x, y)`.
