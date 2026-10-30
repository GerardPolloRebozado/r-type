# Lua cookbook

## Gravity and jump

```lua
local Player = {}
local SPEED = 240
local GRAVITY = 900
local JUMP = -420

function Player:onCreate(entity)
    self.entity = entity
    self.vy = 0
    self.grounded = false
    Component.connect(entity, "onCollisionEnter", entity, "onCollisionEnter")
    Component.connect(entity, "onCollisionStay", entity, "onCollisionStay")
end

function Player:onUpdate(entity, dt)
    local dir = 0
    if Controller.isActionActive("left") then dir = dir - 1 end
    if Controller.isActionActive("right") then dir = dir + 1 end
    Transform.translate(entity, dir * SPEED * dt, 0)

    self.vy = self.vy + GRAVITY * dt
    if Controller.isActionActive("jump") and self.grounded then
        self.vy = JUMP
        self.grounded = false
    end
    Transform.translate(entity, 0, self.vy * dt)
end

function Player:onCollisionStay(selfEntity, other)
    self.grounded = true
end

return Player
```

## Animation (frame swap)

```lua
local frames = {"assets/p1.png", "assets/p2.png"}
local timer = 0
local frame = 1

function Player:onUpdate(entity, dt)
    timer = timer + dt
    if timer > 0.15 then
        timer = 0
        frame = (frame % #frames) + 1
        Component.setSpritePath(entity, frames[frame])
    end
    if Controller.isActionActive("left") then
        Component.setSpriteFlip(entity, true)
    elseif Controller.isActionActive("right") then
        Component.setSpriteFlip(entity, false)
    end
end
```

## Spawning with a timer

```lua
local EnemySpawner = {}
local timer = 0

function EnemySpawner:onUpdate(entity, dt)
    timer = timer + dt
    if timer > 2.0 then
        timer = 0
        local e = Entity.create()
        Entity.setName(e, "Enemy")
        Transform.setPosition(e, 900, 300)
        Component.addSprite(e, "assets/enemy.png")
        Component.addBoxCollider(e, 32, 32)
    end
end

return EnemySpawner
```

## Simple state machine in Lua

```lua
local Game = {}
Game.state = "menu"

function Game:onUpdate(entity, dt)
    if Game.state == "menu" then
        if Controller.isActionActive("start") then
            Game.state = "playing"
            Scene.load("levels/Level1.scene")
        end
    elseif Game.state == "playing" then
        -- gameplay
    elseif Game.state == "gameover" then
        if Controller.isActionActive("restart") then
            Game.state = "menu"
            Scene.load("levels/Menu.scene")
        end
    end
end

return Game
```

## Timers without blocking

Store a deadline and check it each frame; never use blocking waits.

```lua
self.deadline = Time.time() + 3.0

-- in onUpdate:
if Time.time() >= self.deadline then
    -- fire
    self.deadline = Time.time() + 3.0
end
```
