local Player = {}

Player.speed = 500.0

local IDLE = "assets/player/sprite_003.png"
local DOWN_START = "assets/player/sprite_002.png"
local DOWN_MOVE = "assets/player/sprite_001.png"
local UP_START = "assets/player/sprite_004.png"
local UP_MOVE = "assets/player/sprite_005.png"

local START_DURATION = 0.2

local serverPeer = nil
local myNetId = nil
local lastInput = { up = false, down = false, left = false, right = false, shoot = false }

function Player:onCreate(entity)
    self.entity = entity
    self.animDirection = nil
    self.animElapsed = 0.0
    self.currentSprite = nil

    Component.setSpritePath(entity, IDLE)
    self.currentSprite = IDLE

    Net.on("welcome", function(_, data)
        if type(data) == "table" and data.netId then
            myNetId = data.netId
        end
    end)

    Net.onConnect(function(peer)
        serverPeer = peer
    end)
end

function Player:onUpdate(entity, dt)
    if not dt or dt <= 0 then
        dt = 1.0 / 60.0
    end

    -- read local input
    local up = Controller.isActionActive("UP")
    local down = Controller.isActionActive("DOWN")
    local left = Controller.isActionActive("LEFT")
    local right = Controller.isActionActive("RIGHT")
    local shoot = Controller.isActionActive("SHOOT")

    -- send input every frame (unreliable)
    if serverPeer ~= nil then
        Net.send(serverPeer, "key_press", {
            up = up, down = down, left = left, right = right, shoot = shoot
        }, false)
    end

    -- local animation only (server owns position)
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

    local sprite = IDLE
    if direction ~= nil then
        self.animElapsed = self.animElapsed + dt
        if self.animElapsed < START_DURATION then
            sprite = (direction == "up") and UP_START or DOWN_START
        else
            sprite = (direction == "up") and UP_MOVE or DOWN_MOVE
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