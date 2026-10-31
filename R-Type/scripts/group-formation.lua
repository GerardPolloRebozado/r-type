local DRIFT_SPEED = 130.0 -- pixels per second, leftward
local TRI_AMP = 80.0 -- vertical zigzag half-height in pixels
local TRI_FREQ = 0.9 -- zigzag cycles per second

local Script = {}

function Script:onCreate(entity)
    self.entity = entity
    self.time = 0.5 / TRI_FREQ
    self.baseY = Transform.getPosition(entity).y
    self.offsets = {}

    local origin = Transform.getPosition(entity)
    local children = Component.getChildren(entity) or {}
    for _, child in ipairs(children) do
        local position = Transform.getPosition(child)
        table.insert(self.offsets, {
            id = child,
            dx = position.x - origin.x,
            dy = position.y - origin.y,
        })
    end

    log.info("[" .. tostring(entity) .. "] Formation group with " .. tostring(#self.offsets) .. " links")
end

function Script:onUpdate(entity, dt)
    if not dt or dt <= 0 then
        dt = 1.0 / 60.0
    end

    self.time = self.time + dt

    local phase = (self.time * TRI_FREQ) % 2.0
    local tri = phase < 1.0 and phase or (2.0 - phase) -- 0..1..0
    local x = Transform.getPosition(entity).x - DRIFT_SPEED * dt
    local y = self.baseY + (tri * 2.0 - 1.0) * TRI_AMP

    Transform.setPosition(entity, x, y)
    for _, offset in ipairs(self.offsets) do
        Transform.setPosition(offset.id, x + offset.dx, y + offset.dy)
    end
end

function Script:onDestroy(entity)
    log.info("[" .. tostring(entity) .. "] Script destroyed")
end

return Script
