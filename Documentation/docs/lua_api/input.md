# `Input`

The `Input` namespace polls the keyboard and mouse. Key codes are referenced through the global `KEY` table (e.g. `KEY.W`, `KEY.SPACE`).

## Functions

### `Input.isKeyDown(key)`

Whether a key is currently held down.

**Signature:** `Input.isKeyDown(key: integer) -> boolean`

---

### `Input.isKeyPressed(key)`

Whether a key was pressed this frame (press event).

**Signature:** `Input.isKeyPressed(key: integer) -> boolean`

---

### `Input.isKeyReleased(key)`

Whether a key was released this frame.

**Signature:** `Input.isKeyReleased(key: integer) -> boolean`

**Example**

```lua
if Input.isKeyPressed(KEY.SPACE) then
    shoot()
end
```

---

### `Input.getMousePosition()`

Current mouse cursor position.

**Signature:** `Input.getMousePosition() -> {x: number, y: number}`

**Returns:** a table `{x, y}`.

**Example**

```lua
local mp = Input.getMousePosition()
Transform.setPosition(cursor, mp.x, mp.y)
```

---

## `KEY` constants

The `KEY` table mirrors raylib's `KEY_*` values. Commonly used entries:

| Name | Description |
|------|-------------|
| `KEY.SPACE` | Space bar. |
| `KEY.ENTER` | Enter. |
| `KEY.ESCAPE` | Escape. |
| `KEY.TAB` | Tab. |
| `KEY.A` … `KEY.Z` | Letter keys. |
| `KEY.ZERO` … `KEY.NINE` | Digit keys. |
| `KEY.F1` … `KEY.F12` | Function keys. |
| `KEY.UP`, `KEY.DOWN`, `KEY.LEFT`, `KEY.RIGHT` | Arrow keys. |
| `KEY.LEFT_SHIFT`, `KEY.LEFT_CONTROL`, `KEY.LEFT_ALT` | Modifiers. |

See `Engine/src/Scripting/LuaAPI/Plugins/InputAPI.cpp` for the full mapping.