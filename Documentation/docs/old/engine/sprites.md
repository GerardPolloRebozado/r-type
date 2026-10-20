# Sprite Rendering

Sprites are drawn by `Engine::SpriteSystem` from a `Engine::SpriteComponent`. The subsystem itself is thin: a
component names an asset, places it relative to the entity, and the system draws it once per frame.

---

## `Engine::SpriteComponent`

| Field | Meaning |
|---|---|
| `sprite` | Runtime `std::shared_ptr<Sprite>` (never serialized, reloaded from `assetPath`). |
| `assetPath` | Project-relative path of the image, e.g. `assets/enemies/bink/bink000.png`. |
| `offsetX` / `offsetY` | Offset from the entity position to the sprite's **top-left corner**. |
| `scaleX` / `scaleY` | Size multipliers. |
| `flipX` / `flipY` | Mirror the sprite about its own drawn centre. Purely visual. |

The sprite's top-left corner is drawn at `(Position.x + offsetX, Position.y + offsetY)` — positions are
top-left corners everywhere in the engine, which is why the offset is what centres a padded sprite on its
collider. The `scale` and `rotation` fields are not part of the component: `SpriteSystem` reads `Rotation` from
the same entity and applies `scaleX/scaleY` when drawing.

```lua
Component.addSprite(entity, "assets/enemies/bink/bink000.png", -12.8, -14.0, 4.0, 4.0)
local info = Component.getSpriteInfo(entity) -- path, offsets, scales and flips, nil when there is no sprite
```

---

## Flipping

### Visual flip

`flipX` / `flipY` mirror the drawing about the sprite's own centre: the entity, its collider and its children
do not move. `SpriteSystem` draws the sprite with a negative source extent, which is how Raylib mirrors a
texture, and shifts the draw position by the sprite's width/height so the mirrored sprite lands in the *same*
footprint as the unflipped one instead of sliding to one side.

Both flags are serialized with the scene. Scenes saved before flipping existed have no `flipX`/`flipY` keys at
all and load unflipped, so they render exactly as they used to.

In the editor, the Sprite section of the Inspector has **Flip X** and **Flip Y** checkboxes.

```lua
Component.setSpriteFlip(entity, true)        -- flip horizontally, leave flipY alone
Component.setSpriteFlip(entity, false, true) -- unflip X, flip Y
local flip = Component.getSpriteFlip(entity) -- { flipX = bool, flipY = bool }, nil when there is no sprite
```

### Flipping a whole rig: `Component.flipX`

A character that turns around is rarely a single sprite: it usually carries children that have to end up on
the other side too — a front sensor, a weapon mount, an exhaust. `Component.flipX(entity)` handles both halves
of that in one call:

* it toggles the entity's own `flipX`, so the art faces the other way, and
* it mirrors the entity's **whole child subtree** about the entity's centre, so children swap sides while
  keeping their distance from their parent.

```lua
-- bink.lua: the FrontCollider child rides in front of the walking body, and the
-- walker turns around when that sensor touches a wall.
function Script:TurnAround()
    self.dir = -self.dir
    self.turnLock = TURN_LOCKOUT
    Component.flipX(self.entity)
end
```

It is a **toggle**: call it once per direction change (the current facing can be read back with
`Component.getSpriteFlip`). Flipping twice restores the original layout exactly.

A few details worth knowing:

* **Footprints are mirrored, not anchors.** Positions are top-left corners, so boxes grow rightwards from
  them. Mirroring the anchor of the sensor above would leave it 18.65px in front on the left but inside the
  body on the right; reflecting the *footprint* puts it 8.65px in front of the body edge on both sides.
* **Which footprint** follows the same rule as the collision system: the collider's own shape, else the
  entity's `ShapeComponent`, else the drawn sprite, else the entity position alone for marker entities.
* **Children keep their own flip flags.** Mirroring a rig is a layout operation, not a per-child flip: flip a
  child explicitly if it has art that must face the same way as its body.
* **The hierarchy keeps working afterwards.** Children store world positions and follow their parent's frame
  deltas, so a rig stays mirrored as it walks; only the layout is set once, here.

See [Collision Layers & Masks](collisions.md) for the other half of a walker: the layers and masks that decide
what its sensor can actually touch.
