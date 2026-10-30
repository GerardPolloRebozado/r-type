# Add multiplayer

Minimal checklist:

1. Add a `NetworkComponent` to the entities you want replicated (usually on a prefab).
   Set authority: `Server` for enemies/pickups, `Client` for players, `Shared` for walls.
2. Set `syncTransform` (and `syncSprite` if the sprite changes).
3. Set `prefabPath` to the prefab the other side should instantiate.
4. Guard simulation in scripts:

```lua
local function shouldSimulate()
    if not Net.isNetworked() then return true end
    return Net.isServer()
end

function Enemy:onUpdate(entity, dt)
    if not shouldSimulate() then return end
    -- move, animate, collide
end
```

5. Launch with `--server` / `--client`, or use the editor's Play Multiplayer button.

Players are spawned by the engine from the `Client`-authority template's `prefabPath`; Lua
cannot instantiate prefabs. Read a client's input on the server with `Net.input(entity)`.

See [Engine → Networking](../engine/networking.md), [Editor → Multiplayer](../editor/multiplayer.md),
and [Lua → Net](../lua/net.md).
