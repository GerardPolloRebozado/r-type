-- server.lua
-- Attached to: Server

local Script = {
    isServer = false,
}

-- Animation / movement constants (assets/player)
local IDLE = "assets/player/sprite_003.png"
local DOWN_START = "assets/player/sprite_002.png"
local DOWN_MOVE = "assets/player/sprite_001.png"
local UP_START = "assets/player/sprite_004.png"
local UP_MOVE = "assets/player/sprite_005.png"
local START_DURATION = 0.2
local SPEED = 500.0

local players = {}
local inputByEntity = {}
local animByEntity = {}

-- Latest network input per player entity, global so other scripts can read it
NetInput = NetInput or {}

local function stateOf(entity)
    local p = Transform.getPosition(entity)
    return {
        netId = Net.networkId(entity),
        x = p.x,
        y = p.y,
        rot = Transform.getRotation(entity),
        sprite = Component.getSpritePath(entity),
        scale = Component.getSpriteScale(entity),
    }
end

local function broadcastAll()
    if #players == 0 then
        return
    end
    local world = { players = {} }
    for _, e in ipairs(players) do
        world.players[#world.players + 1] = stateOf(e)
    end
    Net.broadcast("world_state", world, false)
end

local function spawnPlayer(peer)
    local playerEntity = Net.peerEntity(peer)
    if playerEntity == nil then
        return
    end

    Component.addSprite(playerEntity, IDLE, 0, -6, 4, 4)
    Transform.setPosition(playerEntity, 0.0, 0.0)

    local gunEntity = Entity.create()
    Component.setParent(gunEntity, playerEntity)
    Component.addSprite(gunEntity, "assets/player/shoot/muzzleflash_000.png", 0, 0, 4, 4)
    Component.addScript(gunEntity, "scripts/playerGun.lua")

    players[#players + 1] = playerEntity
    inputByEntity[playerEntity] = {
        up = false, down = false, left = false, right = false, shoot = false,
    }
    animByEntity[playerEntity] = { direction = nil, elapsed = 0.0, currentSprite = IDLE }

    broadcastAll()
end

local function despawnPlayer(entity, netId)
    for i = #players, 1, -1 do
        if players[i] == entity then
            table.remove(players, i)
            break
        end
    end
    inputByEntity[entity] = nil
    animByEntity[entity] = nil
    NetInput[entity] = nil

    for _, child in ipairs(Component.getChildren(entity)) do
        Entity.destroy(child)
    end

    if netId ~= nil then
        Net.broadcast("player_left", { netId = netId }, true)
    end
end

function Script:onCreate(entity)
    self.entity = entity
    log.info("[" .. tostring(entity) .. "] Script created")

    local mode = (type(arg) == "table" and arg[1]) or nil
    if mode ~= "--server" then
        return
    end
    self.isServer = true

    Net.startServer(3000)

    Net.onConnect(function(peer)
        spawnPlayer(peer)
    end)

    Net.onDisconnect(function(peer)
        local playerEntity = Net.peerEntity(peer)
        if playerEntity == nil then
            return
        end
        despawnPlayer(playerEntity, Net.networkId(playerEntity))
    end)

    Net.on("key_press", function(peer, data)
        if type(data) ~= "table" then
            return
        end
        local playerEntity = Net.peerEntity(peer)
        if playerEntity ~= nil then
            inputByEntity[playerEntity] = data
        end
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

local function animate(entity, input, dt)
    local a = animByEntity[entity]
    if a == nil then
        return
    end

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

    for _, e in ipairs(players) do
        local input = inputByEntity[e]
        if input then
            NetInput[e] = input
            step(e, input, dt)
            animate(e, input, dt)
        end
    end

    broadcastAll()
end

function Script:onDestroy(entity)
    log.info("[" .. tostring(entity) .. "] Script destroyed")
end

return Script
