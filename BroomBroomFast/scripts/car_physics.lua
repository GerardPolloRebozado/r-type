-- car_physics.lua
-- Top-down car movement shared by the player and the AI cars.
-- Angle is in degrees, 0 = facing up, growing clockwise.

local Car = {}

Car.WIDTH = 71
Car.HEIGHT = 131
Car.SCALE = 0.55

local BRAKE = 1100.0 -- deceleration when braking while moving forward
local REVERSE_ACCEL = 350.0
local MAX_REVERSE = 220.0
local FRICTION = 300.0 -- slow-down when no pedal is pressed
local OFFROAD_FACTOR = 0.4 -- share of top speed kept off the track
local OFFROAD_DRAG = 1200.0 -- how fast the car drops to the off-road top speed
local FULL_GRIP_SPEED = 220.0 -- below this speed the car turns slower

function Car.new(entity, params)
    local pos = Transform.getPosition(entity)
    return {
        entity = entity,
        name = Entity.getName(entity),
        x = pos.x,
        y = pos.y,
        z = pos.z,
        angle = Transform.getRotation(entity) or 0.0,
        speed = 0.0,
        maxSpeed = params.maxSpeed or 680.0,
        accel = params.accel or 520.0,
        turnRate = params.turnRate or 180.0,
        -- race progress, updated by race.lua
        lap = 0,
        nextWaypoint = 1,
        finished = false,
        place = nil,
        finishTime = nil,
    }
end

-- throttle / brake: booleans, steer: -1 (left) .. 1 (right)
function Car.step(car, throttle, brake, steer, dt, onRoad)
    if throttle then
        car.speed = car.speed + car.accel * dt
    elseif brake then
        if car.speed > 0 then
            car.speed = math.max(0, car.speed - BRAKE * dt)
        else
            car.speed = car.speed - REVERSE_ACCEL * dt
        end
    else
        local f = FRICTION * dt
        if math.abs(car.speed) <= f then
            car.speed = 0
        elseif car.speed > 0 then
            car.speed = car.speed - f
        else
            car.speed = car.speed + f
        end
    end

    local top = onRoad and car.maxSpeed or car.maxSpeed * OFFROAD_FACTOR
    if car.speed > top then
        car.speed = math.max(top, car.speed - OFFROAD_DRAG * dt)
    end
    if car.speed < -MAX_REVERSE then
        car.speed = -MAX_REVERSE
    end

    local grip = math.min(1.0, math.abs(car.speed) / FULL_GRIP_SPEED)
    local direction = car.speed >= 0 and 1 or -1
    car.angle = (car.angle + steer * car.turnRate * grip * direction * dt) % 360

    local r = math.rad(car.angle)
    car.x = car.x + math.sin(r) * car.speed * dt
    car.y = car.y - math.cos(r) * car.speed * dt
end

-- Writes the car state to its entity. Sprites rotate around their top-left
-- corner, so the offset moves the sprite to keep (x, y) as the car centre.
function Car.apply(car)
    local a = Car.WIDTH * Car.SCALE / 2
    local b = Car.HEIGHT * Car.SCALE / 2
    local r = math.rad(car.angle)
    local c, s = math.cos(r), math.sin(r)
    Transform.setPosition(car.entity, car.x, car.y, car.z)
    Transform.setRotation(car.entity, car.angle)
    Component.setSpriteOffset(car.entity, -(a * c - b * s), -(a * s + b * c))
end

-- Signed difference between two angles, in -180 .. 180.
function Car.angleDiff(from, to)
    return ((to - from + 540) % 360) - 180
end

-- Heading (same convention as car.angle) pointing from (x1, y1) to (x2, y2).
function Car.headingTo(x1, y1, x2, y2)
    return math.deg(math.atan(x2 - x1, -(y2 - y1))) % 360
end

return Car
