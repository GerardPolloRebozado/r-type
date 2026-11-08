-- credits.lua
-- Credits / license screen. ENTER or ESC goes back to the main menu.

local Keys = require("scripts.keys")

local Credits = {}

function Credits:onCreate(entity)
    self.keys = Keys.new()
    self.leaving = false
end

function Credits:onUpdate(entity, dt)
    self.keys:update()
    if not self.leaving and (self.keys:pressed("accept") or self.keys:pressed("back")) then
        self.leaving = true
        Scene.load("Menu.scene")
    end
end

return Credits
