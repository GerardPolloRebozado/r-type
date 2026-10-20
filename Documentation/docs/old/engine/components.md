# Components

A **component** is a plain data struct attached to an entity. It has no logic and no
`Update()`. Systems read and write it; the component itself does nothing.

```cpp
struct Health {
    int current = 100;
    int max = 100;
    bool invincible = false;
};
```

That is a complete, valid component.

---

## Design rules

1. **Data only.** No getters, setters, or update methods. If it needs behaviour, it belongs
   in a system.
2. **Small and focused.** Prefer `Position` + `Velocity` over one `Transform` that also owns
   scale, tint, and physics. Systems subscribe to exactly what they read.
3. **No ownership.** Avoid `std::shared_ptr`, raw owning pointers, and heavy containers.
   Components are copied, moved, and serialized; keep them cheap and value-like.
4. **Serializable by default.** The editor and scene serializer reflect over public fields.
   Use types the reflection layer understands (`float`, `int`, `bool`, `std::string`, enums,
   and nested value structs).
5. **Stable field names.** They are the keys in `.scene` and `.prefab` files.

---

## Registering a component

Components must exist in the registry before they can be added:

```cpp
registry.RegisterComponent<Health>();
```

The registry registers `Position`, `Rotation`, and `ScreenSpace` itself. Everything else is
registered by a [plugin](plugins.md), which is why most game code never calls
`RegisterComponent` directly.

`RegisterComponent<T>()` is idempotent: calling it twice is harmless.

---

## Creating and editing

```cpp
Engine::Entity enemy = registry.CreateEntity();

registry.AddComponent(enemy, Engine::Health{ .current = 3, .max = 3 });
registry.AddComponent(enemy, Engine::Position(640.0f, 200.0f, 5));

bool hasHealth = registry.HasComponent<Engine::Health>(enemy);
auto& health = registry.GetComponent<Engine::Health>(enemy);
health.current -= 1;

registry.RemoveComponent<Engine::Health>(enemy);
```

`CreateEntity()` already attaches a default `Position` and `Rotation`.

`AddComponent` inserts or replaces. `RemoveComponent` erases. Both are O(1) average.

---

## Built-in components

| Component | Purpose |
|---|---|
| `Position` | World `x`, `y`, draw layer `z`, previous-frame values, `moved` flag |
| `Rotation` | Angle in degrees |
| `ScreenSpace` | Tag: draw in window pixels, ignoring the camera (HUD) |
| `Parent` | Hierarchy link to another entity |
| `Name` | Human-readable label used by the editor and prefabs |
| `Opacity` | Alpha multiplier |
| `SpriteComponent` | Sprite asset path, offsets, scale, flip |
| `ShapeComponent` | Rectangle, circle, or regular polygon |
| `Text` | String, font, size, spacing, colour |
| `CameraComponent` | Zoom, follow target, bounds, shake |
| `ColliderComponent` | Collision shape and layer mask |
| `AudioSource` | Sound or music request |
| `NetworkComponent` | Replication flags and network identity |
| `Input` | Per-entity action state |
| `ScriptComponent` | Lua script path and runtime state |
| `PrefabInstance` | Link back to the prefab that spawned this entity |
| `SignalConnections` | Subscriptions to typed signals |

Most of these are documented on their own page:

- [Sprites](sprites.md) · [Text](text.md) · [Camera](camera.md)
- [Collisions](collisions.md) · [Audio](audio.md) · [Input](input.md)
- [State management](states.md) · [Networking](networking.md)

---

## Tags are components too

A component with no fields is a **tag**. It carries one bit of information: "this entity is
different".

```cpp
struct PlayerTag {};
registry.AddComponent(player, PlayerTag{});
```

`ScreenSpace` is the built-in example: the renderer checks for it and switches to the HUD
camera. Use tags instead of booleans on unrelated components; they compose better.

---

## A note on `Transform`

Older examples in this documentation used a `Transform` component. The engine does not have
one: it uses separate `Position` and `Rotation` components, plus `Parent` for hierarchy.
When porting snippets, replace `Transform{ .x, .y }` with `Position(x, y)`.

---

Next: [Systems](systems.md) → [Plugins](plugins.md).
