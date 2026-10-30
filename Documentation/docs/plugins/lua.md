# Lua plugins

Lua scripts are the supported way to write gameplay without recompiling. A script returns a
table with optional lifecycle functions and uses the engine's Lua API.

## Structure

```lua
local Enemy = {}

function Enemy:onCreate(entity)
    self.entity = entity
end

function Enemy:onUpdate(entity, dt)
    -- runs every frame
end

function Enemy:onDestroy(entity)
end

return Enemy
```

## Capabilities

Scripts can create and destroy entities, read and write components, query input, play audio,
spawn prefabs indirectly through the engine, and call into the network API. They cannot
instantiate `.prefab` files directly; the engine spawns those.

## Auto-reload

`ScriptComponent::autoReload` (default `true`) reloads the script when the file changes.
There is no hot reload in the running process; relaunch the game after saving.

## Packaging

Ship scripts alongside the executable in the project's `scripts/` folder. Paths in
`ScriptComponent` are project-relative, so they resolve the same way in the editor and the
built game.

## See also

- [Lua scripting](../engine/scripting.md)
- [Lua API reference](../lua/index.md)
- [Packaging plugins](packaging.md)
