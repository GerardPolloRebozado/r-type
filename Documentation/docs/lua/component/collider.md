# Collider

## `Component.addBoxCollider`

```lua
Component.addBoxCollider(entity, width, height) -> bool
Component.addBoxCollider(entity, width, height, isTrigger) -> bool
Component.addBoxCollider(entity, width, height, isTrigger, isStatic) -> bool
```

Requires an alive entity. Ensures a `Position` (no rotation). Adds `CollisionEvents`.

## `Component.getColliderSize`

```lua
Component.getColliderSize(entity) -> {w, h, width, height} | nil
```

Both `w`/`h` and `width`/`height` aliases are present.

## `Component.setColliderSize`

```lua
Component.setColliderSize(entity, width, height) -> bool
```

## `Component.getColliderEnabled`

```lua
Component.getColliderEnabled(entity) -> bool | nil
```

## `Component.setColliderEnabled`

```lua
Component.setColliderEnabled(entity, enabled) -> bool
```

## `Component.getColliderTrigger`

```lua
Component.getColliderTrigger(entity) -> bool | nil
```

## `Component.setColliderTrigger`

```lua
Component.setColliderTrigger(entity, isTrigger) -> bool
```

## `Component.getColliderStatic`

```lua
Component.getColliderStatic(entity) -> bool | nil
```

## `Component.setColliderStatic`

```lua
Component.setColliderStatic(entity, isStatic) -> bool
```

## `Component.getColliderLayer`

```lua
Component.getColliderLayer(entity) -> int | nil
```

## `Component.setColliderLayer`

```lua
Component.setColliderLayer(entity, layer) -> bool
Component.setColliderLayer(entity, layerName) -> bool
```

The name variant uses `CollisionLayers::Index` and returns `false` on an unknown name.

## `Component.getColliderLayerName`

```lua
Component.getColliderLayerName(entity) -> string | nil
```

## `Component.getColliderMask`

```lua
Component.getColliderMask(entity) -> uint32 | nil
```

## `Component.setColliderMask`

```lua
Component.setColliderMask(entity, mask) -> bool
```

## `Component.collidersCanCollide`

```lua
Component.collidersCanCollide(a, b) -> bool
```

## `Component.isColliding`

```lua
Component.isColliding(entity) -> bool
```

Returns `false` if no `CollisionSystem` is registered.

## `Component.getCollisions`

```lua
Component.getCollisions(entity) -> Entity[]
```

1-indexed list of entities currently colliding with this one.
