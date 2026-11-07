-- car_select.lua
-- Choose the car colour with LEFT/RIGHT, ENTER starts the race on the chosen track.

local Keys = require("scripts.keys")
local GameState = require("scripts.game_state")

local CarSelect = {}

local CAR_W, CAR_H = 71, 131
local FIRST_X = 360 -- centre of the first car
local STEP_X = 300
local CENTER_Y = 465
local SELECTED_SCALE = 1.4
local LABEL_Y = 700
local LABEL_SIZE = 70
local CURSOR_Y = 610
local CURSOR_HALF_WIDTH = 41

function CarSelect:onCreate(entity)
    self.keys = Keys.new()
    self.leaving = false
    self.time = 0
    self.cursor = Entity.find("Cursor")
    self.label = Entity.find("CarLabel")
    self.cars = {}
    self.index = 1
    for i, color in ipairs(GameState.CARS) do
        self.cars[i] = Entity.find("Car" .. i)
        if color == GameState.car then
            self.index = i
        end
    end
    self:refresh()
end

function CarSelect:placeCar(i, scale, bob)
    local car = self.cars[i]
    if not car then
        return
    end
    local cx = FIRST_X + (i - 1) * STEP_X
    Component.setSpriteScale(car, scale, scale)
    Transform.setPosition(car, cx - CAR_W * scale / 2, CENTER_Y - CAR_H * scale / 2 + bob, 4)
end

function CarSelect:refresh()
    for i = 1, #self.cars do
        self:placeCar(i, i == self.index and SELECTED_SCALE or 1.0, 0)
    end
    local name = string.upper(GameState.CARS[self.index])
    if self.label then
        Component.addText(self.label, name, LABEL_SIZE, 255, 210, 60, 255)
        Transform.setPosition(self.label, 960 - #name * LABEL_SIZE * 0.31, LABEL_Y, 4)
    end
    if self.cursor then
        local cx = FIRST_X + (self.index - 1) * STEP_X
        Transform.setPosition(self.cursor, cx - CURSOR_HALF_WIDTH, CURSOR_Y, 4)
    end
end

function CarSelect:onUpdate(entity, dt)
    self.keys:update()
    -- Small bounce on the selected car.
    self.time = self.time + dt
    self:placeCar(self.index, SELECTED_SCALE, math.sin(self.time * 5) * 6)

    if self.leaving then
        return
    end
    local count = #GameState.CARS
    if self.keys:pressed("left") then
        self.index = (self.index - 2) % count + 1
        self:refresh()
    elseif self.keys:pressed("right") then
        self.index = self.index % count + 1
        self:refresh()
    elseif self.keys:pressed("accept") then
        GameState.car = GameState.CARS[self.index]
        self.leaving = true
        Scene.load(GameState.map)
    elseif self.keys:pressed("back") then
        self.leaving = true
        Scene.load("MapSelect.scene")
    end
end

return CarSelect
