local Script = {}

local COUNTRIES = {
    { name = "England",  flag = { x = 474, y = 169, w = 96, h = 64 }, scene = "england.scene",  secret = "secret_england.scene" },
    { name = "China",    flag = { x = 370, y = 169, w = 96, h = 64 }, scene = "china.scene",    secret = "secret_china.scene" },
    { name = "Thailand", flag = { x = 422, y = 241, w = 96, h = 64 }, scene = "thailand.scene", secret = "secret_thailand.scene" },
    { name = "Japan",    flag = { x = 370, y = 97,  w = 96, h = 64 }, scene = "japan.scene",    secret = "secret_japan.scene" },
    { name = "USA",      flag = { x = 474, y = 97,  w = 96, h = 64 }, scene = "fighters.scene", secret = "secret_usa.scene" },
    { name = "Credits",  flag = { x = 0, y = 0, w = 0, h = 0 }, scene = "credits.scene", credits = true },
}
-- voice announcing the country when the cursor lands on it
local VOICES = {
    England = "assets/sound/england.mp3",
    China = "assets/sound/china.mp3",
    USA = "assets/sound/usa.mp3",
    Japan = "assets/sound/japan.mp3",
    Thailand = "assets/sound/thailand.mp3",
}
-- Game mode popup (entities Popup* in game_menu.scene;
local MODES = {
    { id = "solo",   label = "SOLO" },
    { id = "versus", label = "VERSUS" },
    { id = "online", label = "ONLINE" },
}
local DIFFICULTIES = {
    { id = "easy", label = "EASY", delay = 2.0 },
    { id = "hard", label = "HARD", delay = 0.0 },
}
local POPUP_NAMES = { "PopupTitle", "PopupMode", "PopupDifficulty", "PopupInfo", "PopupHint" }
local PARK_X, PARK_Y = -3000.0, -3000.0
local PANEL = { x = 290, y = 170, w = 700, h = 380 }

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
        if not c.marker or (not c.flagEntity and not c.credits) then
            log.error("[CountryMenu] missing Marker_/Flag_ entity for " .. c.name)
        end
    end
    self.nameText = Entity.find("CountryName")


    self.popup = { open = false, modeIdx = 1, diffIdx = 1, row = 1, held = {}, items = {} }
    for _, name in ipairs(POPUP_NAMES) do
        local e = Entity.find(name)
        local p = e and Transform.getPosition(e)
        if e and p then
            self.popup.items[name] = { entity = e, x = p.x, y = p.y, z = p.z }
            Transform.setPosition(e, PARK_X, PARK_Y, p.z)
        else
            log.error("[CountryMenu] missing popup entity " .. name)
        end
    end
    RoundIntro = false
    if SecretMode then
        local title = Entity.find("Title")
        if title then Component.setText(title, "SECRET MODE - CHOOSE A COUNTRY") end
    end
    -- the intro may already be playing this theme: do not restart it
    if Audio and Audio.playMusic and not Audio.isMusicPlaying(MENU_MUSIC) then
        Audio.playMusic(MENU_MUSIC, 0.6, true)
    end
end


local function pressed(action)
    return Controller.isActionActive(action, 1)
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

function Script:setPopupText(name, text)
    local it = self.popup.items[name]
    if it then Component.setText(it.entity, text) end
end

function Script:refreshPopup()
    local pu = self.popup
    local mode, diff = MODES[pu.modeIdx], DIFFICULTIES[pu.diffIdx]
    local solo = mode.id == "solo"
    self:setPopupText("PopupMode", (pu.row == 1 and "> " or "  ") .. "MODE:  <  " .. mode.label .. "  >")
    if solo then
        self:setPopupText("PopupDifficulty",
            (pu.row == 2 and "> " or "  ") .. "DIFFICULTY:  <  " .. diff.label .. "  >")
    else
        self:setPopupText("PopupDifficulty", "  DIFFICULTY:  (SOLO ONLY)")
    end
    self:setPopupText("PopupInfo", mode.id == "online" and "ONLINE IS NOT AVAILABLE YET" or "")
end

function Script:openPopup(country)
    local pu = self.popup
    pu.open, pu.row, pu.country = true, 1, country
    -- the key that opened the popup is still down: do not treat it as a new press
    for _, action in ipairs({ "Left", "Right", "Jump", "Crouch", "Intro", "LightPunch" }) do
        pu.held[action] = pressed(action)
    end
    for _, it in pairs(pu.items) do
        Transform.setPosition(it.entity, it.x, it.y, it.z)
    end
    pu.panel = Entity.create()
    Component.addShape(pu.panel, 0, PANEL.w, PANEL.h, 0, 0, 1, true, 15, 15, 40, 235)
    Transform.setPosition(pu.panel, PANEL.x, PANEL.y, 20)
    pu.border = Entity.create()
    Component.addShape(pu.border, 0, PANEL.w, PANEL.h, 0, 0, 3, false, 255, 200, 0, 255)
    Transform.setPosition(pu.border, PANEL.x, PANEL.y, 21)
    self:refreshPopup()
end

function Script:closePopup()
    local pu = self.popup
    pu.open = false
    for _, it in pairs(pu.items) do
        Transform.setPosition(it.entity, PARK_X, PARK_Y, it.z)
    end
    for _, key in ipairs({ "panel", "border" }) do
        if pu[key] then Entity.destroy(pu[key]); pu[key] = nil end
    end
end

function Script:startGame()
    local pu = self.popup
    local mode, diff = MODES[pu.modeIdx], DIFFICULTIES[pu.diffIdx]
    if mode.id == "online" then
        log.info("[CountryMenu] online mode is not available yet")
        return
    end

    GameMode = mode.id
    Difficulty = diff.id
    AIDelayMultiplier = diff.delay
    SelectedCountry = { name = pu.country.name }
    log.info(string.format("[CountryMenu] %s: %s / %s", pu.country.name, mode.id, diff.id))
    self.done = true
    self:closePopup()
    if Audio and Audio.stopAllMusic then Audio.stopAllMusic() end
    Scene.load(pu.country.scene)
end

function Script:updatePopup()
    local pu = self.popup
    local function edge(action)
        local now = pressed(action)
        local fired = now and not pu.held[action]
        pu.held[action] = now
        return fired
    end
    local left, right = edge("Left"), edge("Right")
    local up, down = edge("Jump"), edge("Crouch")
    local confirm, back = edge("Intro"), edge("LightPunch")
    local solo = MODES[pu.modeIdx].id == "solo"

    if up or down then
        pu.row = (solo and pu.row == 1) and 2 or 1
        self:refreshPopup()
    end
    if left or right then
        local step = left and -1 or 1
        if pu.row == 1 then
            pu.modeIdx = (pu.modeIdx - 1 + step) % #MODES + 1
            if MODES[pu.modeIdx].id ~= "solo" then pu.row = 1 end
        else
            pu.diffIdx = (pu.diffIdx - 1 + step) % #DIFFICULTIES + 1
        end
        self:refreshPopup()
    end
    if back then
        self:closePopup()
    elseif confirm then
        self:startGame()
    end
end

function Script:choose(country)
    if not country.scene then
        log.info("[CountryMenu] " .. country.name .. " is not available yet")
        return
    end
    log.info("[CountryMenu] selected " .. country.name)
    if country.credits then
        self.done = true
        Scene.load(country.scene) -- keeps the menu music playing
        return
    end
    if SecretMode and country.secret then
        SecretMode = false
        GameMode = "solo"
        SelectedCountry = { name = country.name }
        self.done = true
        if Audio and Audio.stopAllMusic then Audio.stopAllMusic() end
        Scene.load(country.secret)
        return
    end
    self:openPopup(country)
end


function Script:onUpdate(entity, dt)
    if self.done then return end

    if not self.ready then
        self.ready = self:setup()
        self:refresh()
    end

    local left, right, confirm = pressed("Left"), pressed("Right"), pressed("Intro")

    if self.popup.open then
        self:updatePopup()
        self.leftHeld, self.rightHeld, self.confirmHeld = left, right, confirm
        return
    end

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
