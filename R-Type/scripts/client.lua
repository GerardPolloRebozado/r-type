-- client.lua
-- Attached to: Client

local Script = {
    isClient = false,
}

local serverPeer = nil
local lastSprite = {}
local shots = {}

-- Returns the local entity mirroring a network id, creating and registering it
local function resolveRemote(netId, sprite)
    local e = Net.entityFromId(netId)
    if e ~= nil then
        return e
    end
    e = Entity.create()
    Component.addSprite(e, sprite or "assets/player/sprite_003.png", 0, -6, 4, 4)
    Net.track(e, netId)
    return e
end

local function applyState(s)
    local e = resolveRemote(s.netId, s.sprite)

    if s.x and s.y then
        Transform.setPosition(e, s.x, s.y)
    end
    if s.rot then
        Transform.setRotation(e, s.rot)
    end
    if s.sprite and lastSprite[e] ~= s.sprite then
        Component.setSpritePath(e, s.sprite)
        lastSprite[e] = s.sprite
    end
    if s.scale and Component.has(e, "Sprite") then
        Component.setSpriteScale(e, s.scale.x, s.scale.y)
    end
end

function Script:onCreate(entity)
    self.entity = entity
    log.info("[" .. tostring(entity) .. "] Script created")

    local mode = (type(arg) == "table" and arg[1]) or nil
    if mode == "--server" then
        return
    end
    self.isClient = true

    Net.startClient("127.0.0.1", 3000)

    Net.onConnect(function(peer)
        serverPeer = peer
    end)

    Net.on("world_state", function(_, world)
        if type(world) ~= "table" or type(world.players) ~= "table" then
            return
        end
        for _, s in ipairs(world.players) do
            if type(s) == "table" and s.netId then
                applyState(s)
            end
        end
    end)

    Net.on("shoot", function(_, data)
        if type(data) ~= "table" then
            return
        end
        local b = Entity.create()
        Component.addSprite(b, "assets/player/shoot/laserbeam.png", 0, 0, 4, 4)
        Transform.setPosition(b, data.x or 0.0, data.y or 0.0)
        shots[#shots + 1] = { entity = b, timer = 2.0 }
    end)

    Net.on("player_left", function(_, d)
        if type(d) ~= "table" or d.netId == nil then
            return
        end
        local e = Net.entityFromId(d.netId)
        if e ~= nil then
            lastSprite[e] = nil
            Net.untrack(e)
            Entity.destroy(e)
        end
    end)
end

function Script:onUpdate(entity, dt)
    if not self.isClient then
        return
    end
    if serverPeer == nil then
        return
    end

    local stepDt = (dt and dt > 0) and dt or (1.0 / 60.0)

    for i = #shots, 1, -1 do
        local s = shots[i]
        Transform.translate(s.entity, 1800.0 * stepDt, 0)
        s.timer = s.timer - stepDt
        if s.timer <= 0 then
            Entity.destroy(s.entity)
            table.remove(shots, i)
        end
    end

    Net.send(serverPeer, "key_press", {
        up = Controller.isActionActive("UP"),
        down = Controller.isActionActive("DOWN"),
        left = Controller.isActionActive("LEFT"),
        right = Controller.isActionActive("RIGHT"),
        shoot = Controller.isActionActive("SHOOT"),
    }, false)
end

function Script:onDestroy(entity)
    log.info("[" .. tostring(entity) .. "] Script destroyed")
end

return Script
