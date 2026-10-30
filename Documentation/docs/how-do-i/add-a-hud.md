# Add a HUD

A HUD is drawn on top of the world, in window pixels, ignoring the camera.

1. Create an entity and add a `Text` component.
2. Add the `ScreenSpace` tag so it is drawn in the screen pass.

```lua
local hud = Entity.create()
Entity.setName(hud, "ScoreHUD")
Transform.setPosition(hud, 16, 16)
Component.addText(hud, "Score: 0", 24, 255, 255, 255, 255)
-- tag it ScreenSpace from C++ or via a prefab; in Lua you can store the entity id
-- and update the text each frame
```

Update the text every frame from your game script:

```lua
function Game:onUpdate(entity, dt)
    local score = self.score or 0
    -- find the HUD entity and update its text
    local hud = Entity.find("ScoreHUD")
    if hud then
        -- Text component has no Lua setter for the string yet; recreate or use a
        -- stored reference. For simple cases, keep the entity id in self.
    end
end
```

For a value that changes often, keep the entity id in `self` during `onCreate` and update
its `Text` component from C++ or by recreating the entity. Sprites and shapes work the same
way: anything tagged `ScreenSpace` is drawn in the screen pass.

See [Engine → Rendering](../engine/rendering.md) for the two-pass pipeline and
[Components → Text](../engine/components.md).
