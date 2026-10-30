# Handle collisions

## React to a hit in Lua

Connect the collision signal to a method on your script:

```lua
function Enemy:onCreate(entity)
    self.entity = entity
    Component.connect(entity, "onCollisionEnter", entity, "onHit")
end

function Enemy:onHit(selfEntity, other)
    log.info("hit by " .. tostring(other))
end
```

## Trigger zones

A collider with `isTrigger = true` reports overlaps without blocking. Use triggers for
pickups, goals, and hit boxes:

```lua
Component.addBoxCollider(entity, 32, 32, true) -- isTrigger = true
Component.connect(entity, "onTriggerEnter", entity, "onPickup")
```

## Layers

Restrict what collides with what using layers and masks:

```lua
Component.setColliderLayer(entity, "Player")
Component.setColliderMask(entity, Layers.bit("Enemy") | Layers.bit("Wall"))
```

See [Engine → Collisions](../engine/collisions.md), [Lua → Collider](../lua/component/collider.md),
and [Lua → Layers](../lua/layers.md).
