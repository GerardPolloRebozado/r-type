-- player.lua
-- The player's car: UP accelerates, DOWN brakes / reverses, LEFT/RIGHT steer.

local Car = require("scripts.car_physics")

local Player = {}

local MAX_SPEED = 700
local ACCEL = 560
local TURN_RATE = 190

function Player:onCreate(entity)
    self.car = nil
end

function Player:onUpdate(entity, dt)
    local rs = RaceState
    if not rs or not rs.ready then
        return
    end
    if not self.car then
        self.car = Car.new(entity, { maxSpeed = MAX_SPEED, accel = ACCEL, turnRate = TURN_RATE })
        self.car.isPlayer = true
        table.insert(rs.cars, self.car)
        rs.player = self.car
    end
    local car = self.car

    -- Locked during the countdown, coasts to a stop after the finish line.
    local canDrive = rs.phase == "race" and not car.finished
    local throttle = canDrive and Controller.isActionActive("UP")
    local brake = canDrive and Controller.isActionActive("DOWN")
    local steer = 0
    if canDrive or car.finished then
        if Controller.isActionActive("LEFT") then
            steer = steer - 1
        end
        if Controller.isActionActive("RIGHT") then
            steer = steer + 1
        end
    end

    Car.step(car, throttle, brake, steer, dt, rs.isOnRoad(car.x, car.y))
    rs.clamp(car)
    Car.apply(car)
end

return Player
