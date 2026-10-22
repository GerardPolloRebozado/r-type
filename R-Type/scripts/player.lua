-- player.lua
-- Attached to: Player prefab (networked, authority = Client)
--
-- One script, three modes:
--   - Single-player : reads the local controller, moves and animates locally.
--   - Server        : reads the input received from the owning client through
--                     Net.input(entity), moves and animates authoritatively.
--   - Network client: does nothing; the server position/sprite is applied by
--                     ReplicationSystem. This avoids rubber-banding.
--
-- Shooting lives in playerGun.lua (the Gun child).

Player = {}

Player.speed = 500.0

-- Every player owns a block of 5 sprites, one block per color. The server
-- assigns each connected client a distinct slot (0-based) and player.lua picks
-- the matching block:
--   slot 0 -> sprite_001..005, slot 1 -> 006..010, ... slot 4 -> 021..025.
-- Inside a block: +0 down-move, +1 down-start, +2 idle, +3 up-start, +4 up-move.
local SPRITES_PER_SLOT = 5

local function SpritePath(base, offset)
    return string.format("assets/player/sprite_%03d.png", base + offset)
end

local function SpritesForSlot(slot)
    local base = slot * SPRITES_PER_SLOT + 1
    return {
        idle = SpritePath(base, 2),
        downStart = SpritePath(base, 1),
        downMove = SpritePath(base, 0),
        upStart = SpritePath(base, 3),
        upMove = SpritePath(base, 4),
    }
end

-- Color slot assigned by the server. Single-player (and the first client tick
-- before the spawn packet is applied) falls back to slot 0.
local function ReadSlot(entity)
    if Net ~= nil and Net.playerSlot ~= nil then
        local slot = Net.playerSlot(entity)
        if slot ~= nil then
            return math.floor(slot)
        end
    end
    return 0
end

local START_DURATION = 0.2

local function isNetworked()
    return Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked()
end

-- Only the single-player process and the server simulate the player.
local function shouldSimulate()
    if not isNetworked() then
        return true
    end
    return Net.isServer()
end

-- Prefer the replicated input (server) / local input (client): a set-table
-- keyed by action name, e.g. { UP = true, SHOOT = true }. Falls back to
-- the controller in single-player or when Net.input has no data.
local function readInput(entity)
    if Net ~= nil and Net.input ~= nil then
        local input = Net.input(entity)
        if input ~= nil then
            return input
        end
    end
    local input = {}
    if Controller.isActionActive("UP") then
        input.UP = true
    end
    if Controller.isActionActive("DOWN") then
        input.DOWN = true
    end
    if Controller.isActionActive("LEFT") then
        input.LEFT = true
    end
    if Controller.isActionActive("RIGHT") then
        input.RIGHT = true
    end
    if Controller.isActionActive("SHOOT") then
        input.SHOOT = true
    end
    return input
end

function Player:onCreate(entity)
    self.entity = entity
    self.animDirection = nil
    self.animElapsed = 0.0
    self.slot = ReadSlot(entity)
    self.sprites = SpritesForSlot(self.slot)
    self.currentSprite = nil

    Component.setSpritePath(entity, self.sprites.idle)
    self.currentSprite = self.sprites.idle
end

function Player:onUpdate(entity, dt)
    -- Networked client: the server owns position and animation for every
    -- player (including this one). ReplicationSystem applies them.
    if not shouldSimulate() then
        return
    end

    if not dt or dt <= 0 then
        dt = 1.0 / 60.0
    end

    -- The slot is assigned by the server around spawn time; refresh in case it
    -- was not known yet on the first tick.
    local slot = ReadSlot(entity)
    if slot ~= self.slot then
        self.slot = slot
        self.sprites = SpritesForSlot(slot)
        self.currentSprite = nil
    end

    local input = readInput(entity)
    local up = input.UP == true
    local down = input.DOWN == true
    local left = input.LEFT == true
    local right = input.RIGHT == true

    -- Movement
    local dx, dy = 0.0, 0.0
    if left then
        dx = dx - 1.0
    end
    if right then
        dx = dx + 1.0
    end
    if up then
        dy = dy - 1.0
    end
    if down then
        dy = dy + 1.0
    end

    if dx ~= 0.0 or dy ~= 0.0 then
        local len = math.sqrt(dx * dx + dy * dy)
        Transform.translate(entity, (dx / len) * Player.speed * dt, (dy / len) * Player.speed * dt)
    end

    -- Animation
    local direction = nil
    if up and not down then
        direction = "up"
    elseif down and not up then
        direction = "down"
    end

    if direction ~= self.animDirection then
        self.animDirection = direction
        self.animElapsed = 0.0
    end

    local sprite = self.sprites.idle
    if direction ~= nil then
        self.animElapsed = self.animElapsed + dt
        if self.animElapsed < START_DURATION then
            sprite = (direction == "up") and self.sprites.upStart or self.sprites.downStart
        else
            sprite = (direction == "up") and self.sprites.upMove or self.sprites.downMove
        end
    end

    if sprite ~= self.currentSprite then
        Component.setSpritePath(entity, sprite)
        self.currentSprite = sprite
    end
end

function Player:collisionTest(one, two)
    log.info("Connection!")
end

function Player:onDestroy(entity)
    log.info("[player.lua] onDestroy entity " .. tostring(entity))
end

return Player
