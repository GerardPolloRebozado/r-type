local Script = {}

local SHEET = "assets/scenarios/Street_Fighter_assets_sin_fondo/Street_Fighter_Intro_transparente.png"
local NEXT_SCENE = "game_menu.scene"

local SCREEN_W, SCREEN_H = 1280, 720
local FRAME_W, FRAME_H = 384, 224
local BG_SCALE_X = SCREEN_W / FRAME_W
local BG_SCALE_Y = SCREEN_H / FRAME_H

-- Wall breaking: (full 384x224 pictures).
local WALL = {
    { x = 4,   y = 6,   width = 384, height = 224, time = 0.8 },
    { x = 390, y = 6,   width = 384, height = 224, time = 0.25 },
    { x = 776, y = 6,   width = 384, height = 224, time = 0.15 },
    { x = 4,   y = 233, width = 384, height = 224, time = 0.15 },
    { x = 390, y = 233, width = 384, height = 224, time = 0.15 },
    { x = 776, y = 233, width = 384, height = 224, time = 0.15 },
    { x = 4,   y = 461, width = 384, height = 224, time = 0.6 },
}

local LOGO = { x = 730, y = 460, width = 300, height = 120 }
local LOGO_SCALE = 1.8

-- Punch of the wall.
local FIST = {
    { x = 434, y = 593, width = 122, height = 80 },
    { x = 582, y = 599, width = 104, height = 79 },
    { x = 702, y = 601, width = 88,  height = 76 },
    { x = 816, y = 603, width = 112, height = 72 },
}
local FIST_SCALE = 3.0
local HOLE_X = 195 * BG_SCALE_X
local HOLE_Y = 125 * BG_SCALE_Y
local FIST_FRAME_TIME = 0.2

-- no dedicated intro track: the wall break gets an impact sound and the
-- title uses the menu theme, which keeps playing into the country menu
local IMPACT_SOUND = "assets/sound/classic_punch.mp3"
local THEME = "assets/sound/02 Stage Select.mp3"
local THEME_VOLUME = 0.6

local FADE_TIME = 0.6
local END_HOLD = 2.5

local function makeSprite(x, y, z, rect, scale, copy)
    local e = Entity.create()
    Transform.setPosition(e, x, y, z)
    Component.addSprite(e, string.rep("./", copy) .. SHEET, 0.0, 0.0, scale, scale)
    Component.setSpriteRect(e, rect.x, rect.y, rect.width, rect.height)
    return e
end

function Script:onCreate(entity)
    self.entity = entity
    self.t = 0.0
    self.wallIndex = 1
    self.wallTimer = 0.0
    self.fistIndex = 0
    self.fistTimer = 0.0
    self.done = false

    -- Background (wall)
    self.bg = makeSprite(0, 0, 10, WALL[1], 1.0, 0)
    Component.setSpriteScale(self.bg, BG_SCALE_X, BG_SCALE_Y)

    -- Logo, hidden until the wall is broken
    local logoW = LOGO.width * LOGO_SCALE
    local logoH = LOGO.height * LOGO_SCALE
    self.logo = makeSprite((SCREEN_W - logoW) / 2, 10, 30, LOGO, LOGO_SCALE, 1)
    Component.setOpacity(self.logo, 0.0)

    local f = FIST[1]
    self.fist = makeSprite(HOLE_X - f.width * FIST_SCALE / 2, HOLE_Y - f.height * FIST_SCALE / 2, 5, f, FIST_SCALE, 2)

    log.info("[Intro] started")
end

function Script:finish()
    if self.done then return end
    self.done = true
    Scene.load(NEXT_SCENE)
end

function Script:onUpdate(entity, dt)
    if self.done then return end
    self.t = self.t + dt

    if Controller and Controller.isActionActive and Controller.isActionActive("Jump", 1) then
        self:finish()
        return
    end

    -- wall breaking animation
    if self.wallIndex < #WALL then
        self.wallTimer = self.wallTimer + dt
        if self.wallTimer >= WALL[self.wallIndex].time then
            self.wallTimer = 0.0
            self.wallIndex = self.wallIndex + 1
            if self.wallIndex == 2 and Audio and Audio.playSound then
                Audio.playSound(IMPACT_SOUND, 1.0, false) -- the wall breaks
            end
            local fr = WALL[self.wallIndex]
            Component.setSpriteRect(self.bg, fr.x, fr.y, fr.width, fr.height)
        end
        return
    end

    -- wall is broken, fade in the logo then the character
    self.wallTimer = self.wallTimer + dt
    local logoAlpha = math.min(1.0, math.max(0.0, (self.wallTimer - WALL[#WALL].time) / FADE_TIME))
    Component.setOpacity(self.logo, logoAlpha)
    if logoAlpha > 0.0 and not self.themeStarted and Audio and Audio.playMusic then
        self.themeStarted = true
        Audio.playMusic(THEME, THEME_VOLUME, true)
    end

    if logoAlpha >= 1.0 then
        self.fistTimer = self.fistTimer + dt
        if self.fistIndex == 0 then
            self.fistIndex = 1
        elseif self.fistTimer >= FIST_FRAME_TIME and self.fistIndex < #FIST then
            self.fistTimer = 0.0
            self.fistIndex = self.fistIndex + 1
            local fr = FIST[self.fistIndex]
            Component.setSpriteRect(self.fist, fr.x, fr.y, fr.width, fr.height)
            Transform.setPosition(self.fist, HOLE_X - fr.width * FIST_SCALE / 2,
                HOLE_Y - fr.height * FIST_SCALE / 2, 5)
        elseif self.fistIndex >= #FIST then
            self.endTimer = (self.endTimer or 0.0) + dt
            if self.endTimer >= END_HOLD then
                self:finish()
            end
        end
    end
end

function Script:onDestroy(entity)
    for _, e in ipairs({ self.bg, self.logo, self.fist }) do
        if e and Entity and Entity.destroy then Entity.destroy(e) end
    end
end

return Script
