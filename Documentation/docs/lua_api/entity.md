# `Entity`

The `Entity` namespace creates and queries entities in the ECS registry.

All functions take and return an `Entity` handle (a numeric ID usable as a table key).

## Functions

### `Entity.new(name?)`

Wrap an existing numeric ID as an `Entity`, or create a fresh one and optionally name it.

**Signature:** `Entity.new(idOrEntity?: number | Entity) -> Entity`
**Signature:** `Entity.new(name: string) -> Entity`

**Parameters**

| Name | Type | Description |
|------|------|-------------|
| `idOrEntity` | `number` \| `Entity` (optional) | Existing ID to wrap. |
| `name` | `string` (optional) | Name assigned to the newly created entity. |

**Returns:** `Entity`

---

### `Entity.create()`

Create a new, unnamed entity.

**Signature:** `Entity.create() -> Entity`

**Returns:** `Entity`

**Example**

```lua
local e = Entity.create()
```

---

### `Entity.destroy(entity)`

Destroy an entity and remove all of its components.

**Signature:** `Entity.destroy(entity: Entity)`

**Parameters**

| Name | Type | Description |
|------|------|-------------|
| `entity` | `Entity` | Entity to destroy. |

---

### `Entity.find(name)`

Find the first entity whose name matches `name`.

**Signature:** `Entity.find(name: string) -> Entity | nil`

**Parameters**

| Name | Type | Description |
|------|------|-------------|
| `name` | `string` | Name to look up. |

**Returns:** `Entity` or `nil` when no match exists.

**Example**

```lua
local player = Entity.find("player")
if player then log.info("found player") end
```

---

### `Entity.getName(entity)`

Get the name attached to an entity.

**Signature:** `Entity.getName(entity: Entity) -> string`

---

### `Entity.setName(entity, name)`

Set (or change) the name attached to an entity.

**Signature:** `Entity.setName(entity: Entity, name: string)`

**Example**

```lua
Entity.setName(e, "enemy")
```