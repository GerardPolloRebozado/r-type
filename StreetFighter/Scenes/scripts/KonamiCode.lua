--   Up Up Down Down Left Right Left Right P O
local Script = {}
local SEQUENCE = { "Jump", "Jump", "Crouch", "Crouch", "Left", "Right", "Left", "Right", "ThreePunch", "HeavyPunch" }
local WATCHED = { "Jump", "Crouch", "Left", "Right", "ThreePunch", "HeavyPunch" }
local MAX_GAP = 1.2
local MENU_SCENE = "game_menu.scene"
local CONFIRM_SOUND = "assets/sound/fight.mp3"
local LOAD_DELAY = 0.8

function Script:onCreate(entity)
    self.entity = entity
    self.progress = 0
    self.gap = 0.0
    self.held = {}
    self.exitTimer = nil

    for _, action in ipairs(WATCHED) do
        self.held[action] = Controller.isActionActive(action, 1)
    end
end

function Script:onUpdate(entity, dt)
    if self.exitTimer then
        self.exitTimer = self.exitTimer - dt
        if self.exitTimer <= 0.0 then
            self.exitTimer = nil
            Scene.load(MENU_SCENE)
        end
        return
    end

    if self.progress > 0 then
        self.gap = self.gap + dt
        if self.gap > MAX_GAP then self.progress = 0 end
    end

    for _, action in ipairs(WATCHED) do
        local now = Controller.isActionActive(action, 1)
        if now and not self.held[action] then
            if action == SEQUENCE[self.progress + 1] then
                self.progress = self.progress + 1
                self.gap = 0.0
            elseif action == SEQUENCE[1] then
                self.progress, self.gap = 1, 0.0
            else
                self.progress = 0
            end
            if self.progress == #SEQUENCE then
                self:unlock()
            end
        end
        self.held[action] = now
    end
end

function Script:unlock()
    log.info("[Konami] secret mode unlocked")
    SecretMode = true
    self.progress = 0
    if Audio and Audio.stopAllMusic then
        Audio.stopAllMusic()
        Audio.playSound(CONFIRM_SOUND, 1.0, false)
    end
    self.exitTimer = LOAD_DELAY
end

return Script
