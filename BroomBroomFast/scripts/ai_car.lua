-- ai_car.lua
-- Computer car: drives towards the next waypoint and slows down for sharp turns.

local Car = require("scripts.car_physics")

local AiCar = {}

-- Each AI gets a slightly different top speed so they spread out.
local SPEEDS = { 600, 575, 550 }
local ACCEL = 480
local TURN_RATE = 200
local LANE_SPREAD = 45 -- random offset around the track centre line

function AiCar:onCreate(entity)
    self.car = nil
end

function AiCar:newTarget()
    self.offsetX = (math.random() * 2 - 1) * LANE_SPREAD
    self.offsetY = (math.random() * 2 - 1) * LANE_SPREAD
    self.targetIndex = self.car.nextWaypoint
end

function AiCar:onUpdate(entity, dt)
    local rs = RaceState
    if not rs or not rs.ready then
        return
    end
    if not self.car then
        local n = tonumber(string.match(Entity.getName(entity), "%d+")) or 1
        self.car = Car.new(entity, {
            maxSpeed = (SPEEDS[n] or 560) + math.random() * 30,
            accel = ACCEL,
            turnRate = TURN_RATE,
        })
        table.insert(rs.cars, self.car)
        self:newTarget()
    end
    local car = self.car

    local throttle, brake, steer = false, false, 0
    if rs.phase ~= "countdown" then
        if self.targetIndex ~= car.nextWaypoint then
            self:newTarget()
        end
        local wp = rs.waypoints[car.nextWaypoint]
        local heading = Car.headingTo(car.x, car.y, wp.x + self.offsetX, wp.y + self.offsetY)
        local diff = Car.angleDiff(car.angle, heading)
        steer = math.max(-1, math.min(1, diff / 25))
        throttle = math.abs(diff) < 40 or car.speed < 250
        brake = math.abs(diff) > 75 and car.speed > 300
    end

    Car.step(car, throttle, brake, steer, dt, rs.isOnRoad(car.x, car.y))
    rs.clamp(car)
    Car.apply(car)
end

return AiCar
