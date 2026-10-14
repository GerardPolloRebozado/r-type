# `Component`

The `Component` namespace attaches, reads, queries and removes components on an entity.

## Functions

### `Component.addSprite(entity, path, offsetX?, offsetY?, scaleX?, scaleY?, tint?, rotation?, visible?, layer?)`

Attach a `SpriteComponent` resolved through `AssetManager`.

**Signature:** `Component.addSprite(entity, path, offsetX = 0, offsetY = 0, scaleX = 1, scaleY = 1, tint = {255,255,255,255}, rotation = 0, visible = true, layer = 0)`

**Parameters**

| Name | Type | Description |
|------|------|-------------|
| `entity` | `Entity` | Target entity. |
| `path` | `string` | Sprite asset path / alias. |
| `offsetX`, `offsetY` | `number` | Draw offset. |
| `scaleX`, `scaleY` | `number` | Scale factors. |
| `tint` | `table` | RGBA color table. |
| `rotation` | `number` | Sprite rotation. |
| `visible` | `boolean` | Visibility flag. |
| `layer` | `integer` | Render layer. |

---

### `Component.getSprite(entity)`

Read the sprite attached to an entity (returns `nil` when none).

**Signature:** `Component.getSprite(entity: Entity) -> table?`

**Returns:** a table `{offsetX, offsetY, scaleX, scaleY, rotation, visible, layer, sizeX?, sizeY?, path?}` or `nil`.

---

### `Component.addShape(entity, shapeType, width?, height?, radius?, sides?, filled?, color?)`

Attach a `ShapeComponent`.

**Signature:** `Component.addShape(entity, shapeType, width = 50, height = 50, radius = 25, sides = 3, filled = true, color = {255,255,255,255})`

`shapeType` is one of: `0 = RECTANGLE`, `1 = CIRCLE`, `2 = POLYGON`, `3 = LINE`.

---

### `Component.addText(entity, text, fontSize?, color?, offsetX?, offsetY?, spacing?)`

Attach a `TextComponent`.

**Signature:** `Component.addText(entity, text, fontSize = 24, color = {255,255,255,255}, offsetX = 0, offsetY = 0, spacing = 1)`

---

### `Component.has(entity, typeName)`

Whether an entity has a component of the given type name.

**Signature:** `Component.has(entity: Entity, typeName: string) -> boolean`

Supported type names: `"SpriteComponent"`, `"ShapeComponent"`, `"TextComponent"`, `"Collider"`, `"Position"`, `"Rotation"`.

---

### `Component.remove(entity, typeName)`

Remove a component of the given type name from an entity.

**Signature:** `Component.remove(entity: Entity, typeName: string)`

**Example**

```lua
Component.addSprite(e, "assets/player.png")
if Component.has(e, "SpriteComponent") then
    local s = Component.getSprite(e)
    log.info("loaded at " .. (s.path or "?"))
end
```