-- level_complete.lua
-- Attached to: LevelManager (one per level scene)
--
-- Monitors the camera. When it reaches the end of the level, spawns a big
-- "NEXT STAGE" text, waits a few seconds, then returns to the level select
-- menu (menu.scene).

local LevelComplete = {}

-- End-of-level camera X per scene. The camera pans right; once it passes this
-- value the level is considered complete.
local END_X_BY_SCENE = {
    ["Level1.scene"] = 2200.0,
    ["Level2.scene"] = 4300.0,
    ["Level3.scene"] = 6100.0,
}

local SHOW_TIME = 3.0   -- seconds to show "NEXT STAGE"
local NEXT_SCENE = "level_selector.scene"

function LevelComplete:onCreate(entity)
    self.entity = entity
    self.cameraEntity = Entity.find("Camera")
    self.completed = false
    self.timer = 0.0
    self.textEntity = nil

    local sceneName = Scene.current() or ""
    self.endX = END_X_BY_SCENE[sceneName] or 6000.0
    log.info(string.format("[level_complete] scene=%s endX=%.1f", sceneName, self.endX))
end

function LevelComplete:onUpdate(entity, dt)
    if not dt or dt <= 0 then
        dt = 1.0 / 60.0
    end

    if self.completed then
        self.timer = self.timer + dt
        if self.timer >= SHOW_TIME then
            Scene.load(NEXT_SCENE)
        end
        return
    end

    if self.cameraEntity ~= nil then
        local pos = Transform.getPosition(self.cameraEntity)
        if pos.x >= self.endX then
            self:triggerComplete()
        end
    end
end

function LevelComplete:triggerComplete()
    self.completed = true
    self.timer = 0.0

    -- Spawn big centered "NEXT STAGE" text
    self.textEntity = Entity.create()
    Entity.setName(self.textEntity, "NextStageText")
    Transform.setPosition(self.textEntity, 0, 0, 100)
    Component.addText(self.textEntity, "NEXT STAGE", 120, 255, 255, 255, 255)
    Component.setOpacity(self.textEntity, 1.0)

    log.info("[level_complete] Level complete! NEXT STAGE")
end

return LevelComplete
