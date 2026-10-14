# `Scene`

The `Scene` namespace exposes scene-level operations.

## Functions

### `Scene.load(path)`

Request a scene load. Currently logs the request (the actual scene loading hook is a placeholder for the future file-watcher / hot-reload work).

**Signature:** `Scene.load(path: string)`

**Parameters**

| Name | Type | Description |
|------|------|-------------|
| `path` | `string` | Scene file path. |

**Example**

```lua
Scene.load("levels/level2.scene")
```

---

## Hooks

Script files run inside the Engine's script system. Declare these global functions in your script to receive lifecycle events:

| Hook | Signature | When it runs |
|------|-----------|--------------|
| `onCreate` | `onCreate(dt: number)` | Once when the script is loaded. |
| `onUpdate` | `onUpdate(dt: number)` | Every frame. |
| `onDestroy` | `onDestroy()` | When the entity/script is unloaded. |

See the [Lua Scripting guide](../engine/lua_scripting.md) for the full guide.