local Script = {}

local COUNTRIES = {
    { name = "England",  flag = { x = 474, y = 169, w = 96, h = 64 } },
    { name = "China",    flag = { x = 370, y = 169, w = 96, h = 64 } },
    { name = "Thailand", flag = { x = 422, y = 241, w = 96, h = 64 } },
    { name = "Japan",    flag = { x = 370, y = 97,  w = 96, h = 64 } },
    { name = "USA",      flag = { x = 474, y = 97,  w = 96, h = 64 }, scene = "fighters.scene" },
}
local MAP_CROP = { x = 0, y = 0, w = 356, h = 228 }

function Script:onCreate(entity)
    self.entity = entity
    self.selected = 1
    self.ready = false
    self.leftHeld, self.rightHeld, self.confirmHeld = false, false, false
    self.done = false
    for _, c in ipairs(COUNTRIES) do
        c.marker = Entity.find("Marker_" .. c.name)
        c.flagEntity = Entity.find("Flag_" .. c.name)
        if not c.marker or not c.flagEntity then
            log.error("[CountryMenu] missing Marker_/Flag_ entity for " .. c.name)
        end
    end
    self.nameText = Entity.find("CountryName")
end


function Script:setup()
    local ok = Component.setSpriteRect(self.entity, MAP_CROP.x, MAP_CROP.y, MAP_CROP.w, MAP_CROP.h)
    for _, c in ipairs(COUNTRIES) do
        if c.flagEntity then
            local f = c.flag
            ok = Component.setSpriteRect(c.flagEntity, f.x, f.y, f.w, f.h) and ok
        end
    end
    return ok
end

function Script:refresh()
    for i, c in ipairs(COUNTRIES) do
        local sel = (i == self.selected)
        if c.marker then Component.setOpacity(c.marker, sel and 1.0 or 0.35) end
        if c.flagEntity then Component.setOpacity(c.flagEntity, sel and 1.0 or 0.0) end
    end
    if self.nameText then
        Component.setText(self.nameText, string.upper(COUNTRIES[self.selected].name))
    end
end

function Script:choose(country)
    if not country.scene then
        log.info("[CountryMenu] " .. country.name .. " is not available yet")
        return
    end
    SelectedCountry = { name = country.name }
    log.info("[CountryMenu] selected " .. country.name)
    self.done = true
    Scene.load(country.scene)
end

local function pressed(action)
    return Controller.isActionActive(action, 1)
end

function Script:onUpdate(entity, dt)
    if self.done then return end

    if not self.ready then
        self.ready = self:setup()
        self:refresh()
        return
    end

    local left, right, confirm = pressed("Left"), pressed("Right"), pressed("Intro")

    if left and not self.leftHeld then
        self.selected = (self.selected - 2) % #COUNTRIES + 1
        self:refresh()
    elseif right and not self.rightHeld then
        self.selected = self.selected % #COUNTRIES + 1
        self:refresh()
    end
    if (confirm and not self.confirmHeld) then
        self:choose(COUNTRIES[self.selected])
    end

    self.leftHeld, self.rightHeld, self.confirmHeld = left, right, confirm
    self.clickHeld = click
end

return Script
