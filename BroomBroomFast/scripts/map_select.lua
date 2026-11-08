-- map_select.lua
-- Choose the track (asphalt, dirt or grass) with LEFT/RIGHT and ENTER.

local Keys = require("scripts.keys")
local GameState = require("scripts.game_state")

local MapSelect = {}

local FONT_SIZE = 70
local CARD_X = 400 -- centre of the first card
local CARD_STEP = 560
local LABEL_Y = 650
local CURSOR_Y = 760
local CURSOR_HALF_WIDTH = 41
local SELECTED = { 255, 210, 60 }
local NORMAL = { 255, 255, 255 }

function MapSelect:onCreate(entity)
    self.keys = Keys.new()
    self.leaving = false
    self.cursor = Entity.find("Cursor")
    self.labels = {}
    self.index = 1
    for i, map in ipairs(GameState.MAPS) do
        self.labels[i] = Entity.find("MapLabel" .. i)
        if map.scene == GameState.map then
            self.index = i
        end
    end
    self:refresh()
end

function MapSelect:refresh()
    for i, label in ipairs(self.labels) do
        local c = (i == self.index) and SELECTED or NORMAL
        Component.addText(label, GameState.MAPS[i].label, FONT_SIZE, c[1], c[2], c[3], 255)
    end
    if self.cursor then
        local cx = CARD_X + (self.index - 1) * CARD_STEP
        Transform.setPosition(self.cursor, cx - CURSOR_HALF_WIDTH, CURSOR_Y, 4)
    end
end

function MapSelect:onUpdate(entity, dt)
    self.keys:update()
    if self.leaving then
        return
    end
    local count = #GameState.MAPS
    if self.keys:pressed("left") then
        self.index = (self.index - 2) % count + 1
        self:refresh()
    elseif self.keys:pressed("right") then
        self.index = self.index % count + 1
        self:refresh()
    elseif self.keys:pressed("accept") then
        GameState.map = GameState.MAPS[self.index].scene
        self.leaving = true
        Scene.load("CarSelect.scene")
    elseif self.keys:pressed("back") then
        self.leaving = true
        Scene.load("Menu.scene")
    end
end

return MapSelect
