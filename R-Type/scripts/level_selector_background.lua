-- level_selector_background.lua
-- Shows a stage background while hovering level buttons; defaults to stage 1.

local LevelSelectorBackground = {}

local backgrounds = {}

local function setLevel(level)
    if type(level) ~= "number" then
        level = 1
    end
    if level < 1 or level > #backgrounds then
        level = 1
    end
    for i, bg in ipairs(backgrounds) do
        local alpha = (i == level) and 1.0 or 0.0
        Component.setOpacity(bg, alpha)
    end
end

function LevelSelectorBackground:onCreate(entity)
    self.entity = entity

    for i = 1, 3 do
        local bg = Entity.create()
        Component.addSprite(bg, "assets/stages/stage" .. i .. ".png", 0, 0, 4, 4)
        Component.setOpacity(bg, (i == 1) and 1.0 or 0.0)
        backgrounds[i] = bg
    end

    _G.setLevelBackground = function(level)
        setLevel(level)
    end

    setLevel(1)
end

function LevelSelectorBackground:onUpdate(entity, dt)
end

return LevelSelectorBackground