-- menu_car.lua
-- Decorative car driving across the main menu road.

local MenuCar = {}

local WRAP_RIGHT = 2050
local WRAP_LEFT = -200

function MenuCar:onCreate(entity)
    self.speed = 260 + math.random() * 220
end

function MenuCar:onUpdate(entity, dt)
    local p = Transform.getPosition(entity)
    local x = p.x + self.speed * dt
    if x > WRAP_RIGHT then
        x = WRAP_LEFT
        self.speed = 260 + math.random() * 220
    end
    Transform.setPosition(entity, x, p.y, p.z)
end

return MenuCar
