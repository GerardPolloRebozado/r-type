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
Player.MAX_LIVES = 3
Player.RESPAWN_DELAY = 1.0
Player.INVULNERABILITY_DURATION = 1.0
Player.states = {}
Player.gameOver = false
Player.gameOverHandlerRegistered = false

local DAMAGE_LAYERS = {
    Enemy = true,
    EnemyBullet = true,
}

local PLAYER_LAYER = "Player"
local PLAYER_MASK = Layers.bit("Enemy") + Layers.bit("EnemyBullet") +
        Layers.bit("PlayerBullet") + Layers.bit("Wall")

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

local function isLocalPlayer(entity)
    if not isNetworked() then
        return true
    end
    if Net.isLocal ~= nil then
        return Net.isLocal(entity)
    end
    return Net.localEntity ~= nil and Net.localEntity() == entity
end

local function showGameOver()
    local textEntity = Entity.find("GameOver")
    Component.setOpacity(textEntity, 1.0)
end

local function registerNetworkHandlers()
    if not isNetworked() then
        return
    end

    if not Player.gameOverHandlerRegistered then
        Player.gameOverHandlerRegistered = true
        Net.on("game_over", function()
            Player.gameOver = true
            showGameOver()
        end)
    end
end

local function setActive(entity, active)
    Component.setColliderEnabled(entity, active)
    Component.setOpacity(entity, active and 1.0 or 0.0, true)
end

local function playerEntities()
    local players = {}
    for _, entity in ipairs(Component.entitiesWith("Collider")) do
        if Component.getColliderLayerName(entity) == PLAYER_LAYER then
            table.insert(players, entity)
        end
    end
    return players
end

local function checkGameOver()
    if Player.gameOver then
        return true
    end

    local players = playerEntities()
    if #players == 0 then
        return false
    end

    for _, entity in ipairs(players) do
        local state = Player.states[entity]
        if state == nil or state.lives > 0 then
            return false
        end
    end

    Player.gameOver = true
    log.info("[player.lua] All players have lost their lives; game over")
    if isNetworked() then
        Net.broadcast("game_over", {}, true)
    else
        showGameOver()
    end
    return true
end

local function readInput(entity)
    local input = {
        UP = false,
        DOWN = false,
        LEFT = false,
        RIGHT = false
    }
    local netInput = Net.input(entity)
    if netInput == nil then
        netInput = input
    end
    if Controller.isActionActive("UP") or netInput.UP then
        input.UP = true
    end
    if Controller.isActionActive("DOWN") or netInput.DOWN then
        input.DOWN = true
    end
    if Controller.isActionActive("LEFT") or netInput.LEFT then
        input.LEFT = true
    end
    if Controller.isActionActive("RIGHT") or netInput.RIGHT then
        input.RIGHT = true
    end
    return input
end

function Player:onCreate(entity)
    registerNetworkHandlers()
    if next(Player.states) == nil then
        Player.gameOver = false
    end

    self.entity = entity
    self.animDirection = nil
    self.animElapsed = 0.0
    self.slot = ReadSlot(entity)
    self.sprites = SpritesForSlot(self.slot)
    self.currentSprite = nil
    self.lives = Player.MAX_LIVES
    self.respawnTimer = 0.0
    self.invulnerabilityTimer = 0.0
    self.active = true
    self.dead = false
    self.isLocal = isLocalPlayer(entity)
    self.lastHudLives = nil
    self.lastAppliedActive = self.active

    if Net.sync ~= nil then
        Net.sync(entity, self, { "lives", "active" })
    end

    Component.setSpritePath(entity, self.sprites.idle)
    self.currentSprite = self.sprites.idle
    Player.states[entity] = self
    Component.connect(entity, "onCollisionEnter", entity, "onCollisionEnter")
    Component.setColliderLayer(entity, PLAYER_LAYER)
    Component.setColliderMask(entity, PLAYER_MASK)
end

function Player:onUpdate(entity, dt)
    self.isLocal = isLocalPlayer(entity)

    if self.isLocal and self.lives ~= self.lastHudLives then
        if Heart ~= nil and Heart.setLives ~= nil then
            self.lastHudLives = self.lives
            Heart:setLives(entity, self.lives)
        end
    end

    if not shouldSimulate() and self.active ~= self.lastAppliedActive then
        self.lastAppliedActive = self.active
        setActive(entity, self.active)
    end

    -- Networked client: the server owns position and animation for every
    -- player (including this one). ReplicationSystem applies them.
    if not shouldSimulate() then
        return
    end

    if not dt or dt <= 0 then
        dt = 1.0 / 60.0
    end

    if Player.gameOver then
        return
    end

    if not self.active then
        self.respawnTimer = self.respawnTimer - dt
        local currOpacity = Component.getOpacity(entity)
        Component.setOpacity(entity, currOpacity == 1.0 and 0.0 or 1.0, true)
        if self.respawnTimer <= 0.0 and self.lives > 0 then
            self.active = true
            self.dead = false
            self.invulnerabilityTimer = Player.INVULNERABILITY_DURATION
            setActive(entity, true)
        end
    end

    self.invulnerabilityTimer = math.max(0.0, self.invulnerabilityTimer - dt)

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

function Player:onCollisionEnter(this, other)
    if not shouldSimulate() or not self.active or self.invulnerabilityTimer > 0.0 or Player.gameOver then
        return
    end

    local layer = Component.getColliderLayerName(other)
    if not DAMAGE_LAYERS[layer] then
        return
    end

    self.lives = math.max(0, self.lives - 1)
    self.active = false
    self.dead = self.lives == 0
    self.respawnTimer = self.dead and 0.0 or Player.RESPAWN_DELAY
    setActive(this, false)

    if layer == "EnemyBullet" then
        Entity.queueDestroy(other)
    end

    if self.dead then
        checkGameOver()
    end
end

function Player:onDestroy(entity)
    Player.states[entity] = nil
    log.info("[player.lua] onDestroy entity " .. tostring(entity))
end

return Player
