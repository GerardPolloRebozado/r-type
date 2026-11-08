-- ship.lua
-- Attached to: Ship

local Script = {}

function Script:onCreate(entity)
    self.entity = entity
    log.info("[" .. tostring(entity) .. "] Script created")
end

function Script:onUpdate(entity, dt)
    local pos = Transform.getPosition(entity)
    local x = pos.x - 120 * dt
    if x < -700 then
        x = 700
    end
    Transform.setPosition(entity, x, pos.y, pos.z)
end

function Script:onDestroy(entity)
    log.info("[" .. tostring(entity) .. "] Script destroyed")
end

return Script
