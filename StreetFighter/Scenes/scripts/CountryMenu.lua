local Script = {}

local COUNTRIES = {
    { name = "England",  flag = { x = 474, y = 169, w = 96, h = 64 }, scene = "england.scene" },
    { name = "China",    flag = { x = 370, y = 169, w = 96, h = 64 }, scene = "china.scene" },
    { name = "Thailand", flag = { x = 422, y = 241, w = 96, h = 64 }, scene = "thailand.scene" },
    { name = "Japan",    flag = { x = 370, y = 97,  w = 96, h = 64 }, scene = "japan.scene" },
    { name = "USA",      flag = { x = 474, y = 97,  w = 96, h = 64 }, scene = "fighters.scene" },
}
-- voice announcing the country when the cursor lands on it
local VOICES = {
    England = "assets/sound/england.mp3",
    China = "assets/sound/china.mp3",
    USA = "assets/sound/usa.mp3",
    Japan = "assets/sound/japan.mp3",
    Thailand = "assets/sound/thailand.mp3",
}
local MENU_MUSIC = "assets/sound/02 Stage Select.mp3"
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
    RoundIntro = false
    -- the intro may already be playing this theme: do not restart it
    if Audio and Audio.playMusic and not Audio.isMusicPlaying(MENU_MUSIC) then
        Audio.playMusic(MENU_MUSIC, 0.6, true)
    end
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

function Script:announce()
    local voice = VOICES[COUNTRIES[self.selected].name]
    if voice and Audio and Audio.playSound then
        Audio.playSound(voice, 1.0, false)
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
    if Audio and Audio.stopAllMusic then Audio.stopAllMusic() end
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
    end

    local left, right, confirm = pressed("Left"), pressed("Right"), pressed("Intro")

    if left and not self.leftHeld then
        self.selected = (self.selected - 2) % #COUNTRIES + 1
        self:refresh()
        self:announce()
        log.info("[CountryMenu] selected: " .. COUNTRIES[self.selected].name)
    elseif right and not self.rightHeld then
        self.selected = self.selected % #COUNTRIES + 1
        self:refresh()
        self:announce()
        log.info("[CountryMenu] selected: " .. COUNTRIES[self.selected].name)
    end
    if (confirm and not self.confirmHeld) then
        self:choose(COUNTRIES[self.selected])
    end

    self.leftHeld, self.rightHeld, self.confirmHeld = left, right, confirm
end

return Script
