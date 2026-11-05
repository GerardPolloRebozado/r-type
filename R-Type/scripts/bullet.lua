-- bullet.lua
-- Attached to: Bullet (pooled projectile)
-- On clients, simulates movement locally using _BulletVelocities.

Bullet = {}

local SPEED = 1800.0
local LIFETIME = 2.0

function Bullet:onCreate(entity)
    self.entity = entity
    self.timer = LIFETIME
    self.vx = SPEED
    self.vy = 0.0
end

function Bullet:onUpdate(entity, dt)
    if dt == nil or dt <= 0 then
        dt = 1.0 / 60.0
    end

    -- Only clients simulate; the server remains authoritative via playerGun.
    if Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked() and not Net.isServer() then
        Transform.translate(entity, self.vx * dt, self.vy * dt)
        self.timer = self.timer - dt
        if self.timer <= 0 then
            Pool.release(entity)
        end
    end
end

return Bullet
