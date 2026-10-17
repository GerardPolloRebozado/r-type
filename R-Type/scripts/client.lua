-- client.lua
-- Attached to: Client

local Script = {
    isClient = false
}

local serverPeer = nil
local myNetId = nil
local entityByNet = {}
local lastSprite = {}

function Script:onCreate(entity)
    self.entity = entity
    log.info("[" .. tostring(entity) .. "] Script created")

    local mode = (type(arg) == "table" and arg[1]) or nil
    if mode == "--server" then
        return
    end
    self.isClient = true
    print("Running as client")

    Net.startClient("127.0.0.1", 3000)

    Net.onConnect(function(peer)
        serverPeer = peer
    end)

    Net.on("welcome", function(_, data)
        if type(data) == "table" and data.netId then
            myNetId = data.netId
        end
    end)

    Net.on("world_state", function(_, world)
        if type(world) ~= "table" or type(world.players) ~= "table" then
            return
        end
        for _, s in ipairs(world.players) do
            if type(s) == "table" and s.netId then
                local e = entityByNet[s.netId]
                if e == nil then
                    e = Entity.create()
                    local path = s.sprite or "assets/player/sprite_003.png"
                    Component.addSprite(e, path, 0, -6, 4, 4)
                    entityByNet[s.netId] = e
                end
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
        end
    end)

    Net.on("player_state", function(_, s)
        if type(s) ~= "table" or s.netId == nil then
            return
        end

        local e = entityByNet[s.netId]
        if e == nil then
            e = Entity.create()
            local path = s.sprite or "assets/player/sprite_003.png"
            Component.addSprite(e, path, 0, -6, 4, 4)
            entityByNet[s.netId] = e
        end

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
    end)

    Net.on("player_left", function(_, d)
        if type(d) == "table" and d.netId then
            local e = entityByNet[d.netId]
            if e then
                Entity.destroy(e)
                entityByNet[d.netId] = nil
                lastSprite[e] = nil
            end
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

    -- send input every frame
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