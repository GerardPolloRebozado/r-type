-- server.lua
-- Attached to: Server

local Script = {
    isServer = false
}

-- constants (copied from player.lua for animation if needed)
local IDLE = "assets/player/sprite_003.png"
local DOWN_START = "assets/player/sprite_002.png"
local DOWN_MOVE = "assets/player/sprite_001.png"
local UP_START = "assets/player/sprite_004.png"
local UP_MOVE = "assets/player/sprite_005.png"
local START_DURATION = 0.2
local SPEED = 500.0

-- state tables keyed by netId
local nextNetId = 1
local netIdByPeer = {}    -- peer (lightuserdata) -> netId
local playerByNet = {}    -- netId -> entity
local peerByNet = {}      -- netId -> peer
local inputByNet = {}     -- netId -> {up,down,left,right,shoot}
local animByNet = {}      -- netId -> {direction, elapsed, currentSprite}

local function stateOf(netId, entity)
    local p = Transform.getPosition(entity)
    return {
        netId = netId,
        x = p.x,
        y = p.y,
        rot = Transform.getRotation(entity),
        sprite = Component.getSpritePath(entity),
        scale = Component.getSpriteScale(entity),
    }
end

local function broadcastAll()
    local world = { players = {} }
    for id, e in pairs(playerByNet) do
        world.players[#world.players + 1] = stateOf(id, e)
    end
    Net.broadcast("world_state", world, false)
end

function Script:onCreate(entity)
    self.entity = entity
    log.info("[" .. tostring(entity) .. "] Script created")
    local mode = (type(arg) == "table" and arg[1]) or nil
    if mode ~= "--server" then
        return
    end
    self.isServer = true
    print("Running as server")

    Net.startServer(3000)

    Net.onConnect(function(peer)
        local e = Net.peerEntity(peer)
        if e == nil then
            return
        end

        local id = nextNetId
        nextNetId = nextNetId + 1

        Component.addSprite(e, "assets/player/sprite_003.png", 0, -6, 4, 4)
        Transform.setPosition(e, 0.0, 0.0)

        netIdByPeer[peer] = id
        playerByNet[id] = e
        peerByNet[id] = peer
        inputByNet[id] = { up = false, down = false, left = false, right = false, shoot = false }
        animByNet[id] = { direction = nil, elapsed = 0.0, currentSprite = IDLE }

        Net.send(peer, "welcome", { netId = id }, true)
        broadcastAll()
    end)

    Net.onDisconnect(function(peer)
        local id = netIdByPeer[peer]
        if id == nil then
            return
        end
        netIdByPeer[peer] = nil
        playerByNet[id] = nil
        peerByNet[id] = nil
        inputByNet[id] = nil
        animByNet[id] = nil
        Net.broadcast("player_left", { netId = id }, true)
    end)

    Net.on("key_press", function(peer, data)
        if type(data) ~= "table" then
            return
        end
        local id = netIdByPeer[peer]
        if id == nil then
            return
        end
        inputByNet[id] = data
    end)
end

local function step(entity, input, dt)
    local dx, dy = 0.0, 0.0
    if input.left then
        dx = dx - 1.0
    end
    if input.right then
        dx = dx + 1.0
    end
    if input.up then
        dy = dy - 1.0
    end
    if input.down then
        dy = dy + 1.0
    end

    if dx == 0.0 and dy == 0.0 then
        return
    end

    local len = math.sqrt(dx * dx + dy * dy)
    Transform.translate(entity, (dx / len) * SPEED * dt, (dy / len) * SPEED * dt)
end

local function animate(entity, netId, dt)
    local a = animByNet[netId]
    if not a then
        return
    end

    local input = inputByNet[netId]
    local direction
    if input.up and not input.down then
        direction = "up"
    elseif input.down and not input.up then
        direction = "down"
    end

    if direction ~= a.direction then
        a.direction = direction
        a.elapsed = 0.0
    end

    local sprite = IDLE
    if direction ~= nil then
        a.elapsed = a.elapsed + dt
        if a.elapsed < START_DURATION then
            sprite = (direction == "up") and UP_START or DOWN_START
        else
            sprite = (direction == "up") and UP_MOVE or DOWN_MOVE
        end
    end

    if sprite ~= a.currentSprite then
        Component.setSpritePath(entity, sprite)
        a.currentSprite = sprite
    end
end

function Script:onUpdate(entity, dt)
    if not self.isServer then
        return
    end
    if not dt or dt <= 0 then
        dt = 1.0 / 60.0
    end

    for id, e in pairs(playerByNet) do
        local input = inputByNet[id]
        if input then
            step(e, input, dt)
            animate(e, id, dt)
        end
    end

    broadcastAll()
end

function Script:onDestroy(entity)
    log.info("[" .. tostring(entity) .. "] Script destroyed")
end

return Script