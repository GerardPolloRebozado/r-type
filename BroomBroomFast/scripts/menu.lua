-- menu.lua
-- Main menu: PLAY / CREDITS / QUIT, chosen with UP/DOWN and ENTER.

local Keys = require("scripts.keys")
require("scripts.game_state")

local Menu = {}

local OPTIONS = { "PLAY", "CREDITS", "QUIT" }
local FONT_SIZE = 70
local FIRST_Y = 460
local STEP_Y = 105
local CURSOR_DY = -6
local SELECTED = { 255, 210, 60 }
local NORMAL = { 255, 255, 255 }

function Menu:onCreate(entity)
    self.keys = Keys.new()
    self.index = 1
    self.leaving = false
    self.cursor = Entity.find("Cursor")
    self.items = {}
    for i = 1, #OPTIONS do
        self.items[i] = Entity.find("Option" .. i)
    end
    self:refresh()
end

function Menu:refresh()
    for i, item in ipairs(self.items) do
        local c = (i == self.index) and SELECTED or NORMAL
        Component.addText(item, OPTIONS[i], FONT_SIZE, c[1], c[2], c[3], 255)
    end
    if self.cursor then
        local p = Transform.getPosition(self.cursor)
        Transform.setPosition(self.cursor, p.x, FIRST_Y + (self.index - 1) * STEP_Y + CURSOR_DY, p.z)
    end
end

function Menu:select()
    local choice = OPTIONS[self.index]
    if choice == "PLAY" then
        self.leaving = true
        Scene.load("MapSelect.scene")
    elseif choice == "CREDITS" then
        self.leaving = true
        Scene.load("Credits.scene")
    elseif choice == "QUIT" and Game and Game.quit then
        Game.quit()
    end
end

function Menu:onUpdate(entity, dt)
    self.keys:update()
    if self.leaving then
        return
    end
    if self.keys:pressed("up") then
        self.index = (self.index - 2) % #OPTIONS + 1
        self:refresh()
    elseif self.keys:pressed("down") then
        self.index = self.index % #OPTIONS + 1
        self:refresh()
    elseif self.keys:pressed("accept") then
        self:select()
    end
end

return Menu
