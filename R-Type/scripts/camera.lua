-- camera.lua
-- Auto-pan camera to the right

local Script = {}

local SPEED = 60.0 -- world units (pixels) per second, to the right
local RAMP_TIME = 1.5 -- seconds to ease up to full speed (0.0 = instant)
local STOP_X = nil -- e.g. 4000.0 to stop there; nil = never stop

local Script = {}

function Script:onCreate(entity)
    self.entity = entity
    self.elapsed = 0.0
    self.stopX = STOP_X

    local pos = Transform.getPosition(entity)
    self.startX = pos.x
    log.info(string.format("[camera-pan.lua] entity %s starts at x=%.1f", tostring(entity), pos.x))
end

function Script:onUpdate(entity, dt)
    if not dt or dt <= 0 then
        dt = 1.0 / 60.0
    end

    -- Set by level_complete.lua while a boss is on screen.
    if CameraScrollPaused then
        return
    end

    -- Ease in so the pan does not start with a jolt.
    self.elapsed = self.elapsed + dt
    local speed = SPEED
    if RAMP_TIME > 0.0 then
        speed = SPEED * math.min(1.0, self.elapsed / RAMP_TIME)
    end

    local pos = Transform.getPosition(entity)
    local step = speed * dt

    -- Land exactly on STOP_X instead of overshooting it.
    if self.stopX ~= nil and pos.x + step >= self.stopX then
        step = self.stopX - pos.x
    end

    if step > 0.0 then
        Transform.translate(entity, step, 0.0)
    end
end

function Script:onDestroy(entity)
    log.info("[camera-pan.lua] entity " .. tostring(entity) .. " destroyed")
end

function Script:onTriggerEnter(camera, other)
    print("Destroying entity " .. tostring(other) .. " because it entered camera trigger")
    Entity.queueDestroy(other)
end

return Script