local Script = {}

local FIGHTERS = { "Sagat", "Birdie", "Geki", "Lee", "Guile", "Zangief", "Dee_Jay", "Ken", "Dhalsim", "Cammy", "Blanka" }
local HOME_X, HOME_Y = 560.0, 520.44
local PARK_X, PARK_Y = -3000.0, -3000.0
local KEY_Q, KEY_E = 81, 69

local LAYOUT = {
    Left = 65, Right = 68, Jump = 87, Crouch = 83,
    LightPunch = 85, MediumPunch = 73, HeavyPunch = 79,
    LightKick = 74, MediumKick = 75, HeavyKick = 76,
}

function Script:onCreate(entity)
    self.entity = entity
    self.selected = 1
    self.changed = true
    self.fighters = {}
    for i, name in ipairs(FIGHTERS) do
        self.fighters[i] = Entity.find("FighterP2_" .. name)
        if not self.fighters[i] then
            log.error("[FighterViewer] entity FighterP2_" .. name .. " not found")
        end
    end
    self.label = Entity.find("CharName")
    if Controller and Controller.addKeybind then
        for action, key in pairs(LAYOUT) do
            Controller.addKeybind(action, key, Controller.KEYBOARD, 0, 2)
        end
    end
end

function Script:onUpdate(entity, dt)
    local n = #FIGHTERS
    if Input.isKeyPressed(KEY_Q) then
        self.selected = (self.selected - 2) % n + 1
        self.changed = true
    elseif Input.isKeyPressed(KEY_E) then
        self.selected = self.selected % n + 1
        self.changed = true
    end

    for i, fighter in ipairs(self.fighters) do
        if fighter then
            if i ~= self.selected then
                Transform.setPosition(fighter, PARK_X, PARK_Y)
            elseif self.changed then
                Transform.setPosition(fighter, HOME_X, HOME_Y)
            end
        end
    end

    if self.changed then
        self.changed = false
        if self.label then
            Component.setText(self.label, string.upper((FIGHTERS[self.selected]:gsub("_", " "))) ..
                string.format("   (%d/%d)", self.selected, n))
        end
        log.info("[FighterViewer] " .. FIGHTERS[self.selected])
    end
end

return Script
