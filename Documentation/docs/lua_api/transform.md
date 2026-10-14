# `Transform`

The `Transform` namespace operates on the `Position` and `Rotation` components of an entity. Every function takes an `Entity` as its first argument.

## Functions

### `Transform.getPosition(entity)`

Get the world-space position of an entity.

**Signature:** `Transform.getPosition(entity: Entity) -> {x: number, y: number}`

**Parameters**

| Name | Type | Description |
|------|------|-------------|
| `entity` | `Entity` | Target entity (must have a `Position` component). |

**Returns:** a table `{x, y}`.

**Example**

```lua
local pos = Transform.getPosition(e)
log.info("pos = " .. pos.x .. ", " .. pos.y)
```

---

### `Transform.setPosition(entity, x, y)`

Set the absolute position of an entity.

**Signature:** `Transform.setPosition(entity: Entity, x: number, y: number)`

---

### `Transform.translate(entity, dx, dy)`

Move an entity by a delta.

**Signature:** `Transform.translate(entity: Entity, dx: number, dy: number)`

**Example**

```lua
Transform.translate(player, speed * dt, 0)
```

---

### `Transform.getRotation(entity)`

Get the rotation angle in degrees.

**Signature:** `Transform.getRotation(entity: Entity) -> number`

---

### `Transform.setRotation(entity, angle)`

Set the absolute rotation angle in degrees.

**Signature:** `Transform.setRotation(entity: Entity, angle: number)`

---

### `Transform.rotate(entity, deltaAngle)`

Add to the current rotation angle.

**Signature:** `Transform.rotate(entity: Entity, deltaAngle: number)`

**Example**

```lua
Transform.rotate(barrel, 90 * dt)
```