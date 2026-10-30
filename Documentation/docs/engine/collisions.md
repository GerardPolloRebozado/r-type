# Collisions

The collision subsystem provides 2D collision detection and trigger management. It supports
convex polygons, circles, and rectangles via SAT.

## Shapes

Shapes are defined by `ShapeComponent`:

- `RECTANGLE`  -  four world-space corner vertices, rotated around the centre.
- `CIRCLE`  -  approximated as a 24-sided regular polygon, so it participates directly in
  SAT.
- `POLYGON`  -  N regular vertices spaced by angle steps around the centre (`sides = 3`
  gives a triangle).
- `LINE`  -  a segment.

## Solid vs. trigger

- **Solid colliders** (`isTrigger = false`) are physical barriers. They fire
  `onCollisionEnter`, `onCollisionStay`, and `onCollisionExit`.
- **Trigger zones** (`isTrigger = true`) report overlaps without blocking. They fire
  `onTriggerEnter`, `onTriggerStay`, and `onTriggerExit` (and the collision events).

## ColliderComponent

```cpp
Engine::ColliderComponent col;
col.enabled = true;
col.isTrigger = false;
col.isStatic = false;       // static colliders are not moved during resolution
col.layer = CollisionLayers::kDefaultLayer;
col.mask = CollisionLayers::kAllLayers;
col.shape = std::optional<Engine::ShapeComponent>{};
```

If `shape` is empty, the system looks for a `ShapeComponent` on the same entity. `ColliderPlugin::AttachToEntity` creates the collider and a `CollisionEvents` component.

## Layers and masks

Two colliders interact only when each one's mask contains the other's layer, so either
side can veto a pair without touching the other entity:

```cpp
// Player touches EnemyBullet and Wall only.
collider.layer = CollisionLayers::Bit("Player");
collider.mask = CollisionLayers::Bit("EnemyBullet") | CollisionLayers::Bit("Wall");
```

The filter runs before overlap detection, so a rejected pair skips detection, resolution,
and events. Layers 0–7 have conventional names (`Default`, `Player`, `Enemy`,
`PlayerBullet`, `EnemyBullet`, `Wall`, `Pickup`, `Trigger`); 8–31 are `Layer N`.

Helpers live in `Engine/include/Components/CollisionLayers.hpp`: `Bit`, `Name`, `Index`,
`kAllLayers`, `kNoLayers`, `kDefaultLayer`.

## Events

`CollisionEvents` stores callbacks:

```cpp
Engine::CollisionEvents events;
events.onCollisionEnter.emplace_back([](Engine::Entity self, Engine::Entity other) { });
events.onCollisionExit.emplace_back([](Engine::Entity self, Engine::Entity other) { });
events.onTriggerEnter.emplace_back([](Engine::Entity self, Engine::Entity other) { });
events.onTriggerExit.emplace_back([](Engine::Entity self, Engine::Entity other) { });
registry.AddComponent(entity, events);
```

## Setup

```cpp
Engine::ColliderPlugin colliderPlugin;
colliderPlugin.Init(app.GetRegistry());
```

## Querying contacts

```cpp
auto collisionSystem = registry.GetSystem<Engine::CollisionSystem>();
const auto& contacts = collisionSystem->GetActiveContacts();
```

## Safety

Collisions can fire callbacks during `Update`. Use `QueueDestroyEntity` inside callbacks,
never `DestroyEntity`. `addBoxCollider` in Lua reconfigures an existing collider in place
rather than replacing it, so layer and mask authored in the scene are preserved.

See [Engine → Systems](systems.md) and the `Games/Collisions` example.
