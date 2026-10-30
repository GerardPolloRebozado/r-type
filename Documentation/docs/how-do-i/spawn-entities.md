# Spawn entities at runtime

From Lua, create a bare entity and dress it with components:

```lua
local e = Entity.create()
Entity.setName(e, "Bullet")
Transform.setPosition(e, x, y)
Component.addSprite(e, "assets/bullet.png")
Component.addBoxCollider(e, 8, 8, false, true) -- not a trigger, static-ish
```

Lua cannot instantiate a `.prefab`. For authored subtrees, spawn them from C++ or let the
engine spawn them from a `prefabPath` in a `NetworkComponent` during replication.

When spawning many entities, queue destruction of the spawner's old instances with
`Entity.queueDestroy` rather than `Entity.destroy` inside a loop.

See [Lua → Entity](../lua/entity.md) and [Editor → Prefabs](../editor/prefabs.md).
