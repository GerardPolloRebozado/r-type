# Lua scripting

The engine exposes gameplay scripting through `ScriptComponent` and `ScriptSystem`, using
sol2. A script returns a table with optional lifecycle functions:

```lua
local Enemy = {}

function Enemy:onCreate(entity)
    self.entity = entity
end

function Enemy:onUpdate(entity, dt)
    -- runs every frame
end

function Enemy:onDestroy(entity)
    -- runs when the entity is destroyed
end

return Enemy
```

## Setup

```cpp
Engine::ScriptPlugin scriptPlugin;
scriptPlugin.Init(app.GetRegistry());
```

`ScriptPlugin` registers `ScriptComponent` and `ScriptSystem`. Attach a script:

```cpp
registry.AddComponent(entity, Engine::ScriptComponent("scripts/enemy.lua"));
```

Or use the helper:

```cpp
Engine::ScriptPlugin::AttachToEntity(registry, entity, Engine::ScriptComponent("scripts/enemy.lua"));
```

## Lifecycle

- `onCreate(entity)`  -  once, when the script is loaded.
- `onUpdate(entity, dt)`  -  every frame, with `dt` in seconds.
- `onDestroy(entity)`  -  when the entity is destroyed.
- `onCollisionEnter/Stay/Exit(this, other)`  -  collision callbacks, when connected.

The editor does not run Lua. Scripts only execute in the game process started by **Play**.

## Auto-reload

`ScriptComponent::autoReload` (default `true`) reloads the script when the file changes on
disk. There is no hot reload in the running process; save the file, close the game window,
and click Play again.

## Engine API

Scripts use the global namespaces registered by `LuaAPI`:

- `Entity`  -  create, destroy, queueDestroy, find, getName, setName
- `Transform`  -  getPosition, setPosition, translate, getZIndex, setZIndex, getRotation, setRotation, rotate
- `Component`  -  addSprite, getSprite, addShape, addText, addBoxCollider, flipX, flipY, setSpritePath, setParent, getChildren, and many more
- `Input`  -  isKeyDown, isKeyPressed, isKeyReleased, getMousePosition, plus `KEY.*`
- `Controller`  -  isActionActive, getActiveActions, addKeybind, hasController, plus `Controller.KEYBOARD.*`
- `Time`  -  `Time.time`, `Time.deltaTime`
- `Net`  -  register, on, off, send, broadcast, onConnect, onDisconnect, isServer, localId, replicate, rpc, and more
- `Scene`  -  `Scene.load(name)`, `Scene.current()`
- `log` / `print`  -  logging at info/warn/error/debug levels

See [Reference → Lua API](../lua/index.md) for the full list.

## ScriptComponent fields

| Field | Default | Notes |
|---|---|---|
| `scriptPath` | empty | Path relative to the project folder |
| `autoReload` | `true` | Reload when the file changes |
| `loaded` | `false` | Internal: whether the script loaded successfully |
| `scriptTable` | nil | Internal: sol2 table handle |

## Networking and scripts

In multiplayer, guard simulation with `Net.isNetworked()` and `Net.isServer()` so the same
script runs in single-player, on the server, and on clients without changes. See
[Networking](networking.md).
