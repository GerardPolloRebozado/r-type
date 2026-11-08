-- level_complete.lua
-- Attached to: LevelManager (one per level scene)
--
-- A stage is complete when either:
--   * the player touches the "StageEnd" trigger collider placed at the end
--     of the stage, or
--   * the camera pans past that same StageEnd X (backup).
--
-- When completed, a big "STAGE CLEAR" text is shown and, after SHOW_TIME
-- seconds, the game returns to the level selection menu (level_selector.scene).

local LevelComplete = {}

local SHOW_TIME = 3.0          -- seconds to show "STAGE CLEAR"
local NEXT_SCENE = "level_selector.scene"
local DEFAULT_END_X = 6000.0   -- used only if the scene has no StageEnd
local MESSAGE = "STAGE CLEAR"
local FONT_SIZE = 120
-- Text is drawn in world space from its top-left corner, so it is moved with
-- the camera each frame using this offset to keep it centred on screen.
local TEXT_OFFSET_X = -360.0
local TEXT_OFFSET_Y = -60.0

function LevelComplete:onCreate(entity)
    self.entity = entity
    self.cameraEntity = Entity.find("Camera")
    self.stageEnd = Entity.find("StageEnd")
    self.completed = false
    self.timer = 0.0
    self.textEntity = nil

    self.endX = DEFAULT_END_X
    if self.stageEnd ~= nil then
        self.endX = Transform.getPosition(self.stageEnd).x
        -- When the player touches StageEnd, call this LevelManager's
        -- onStageEndEnter handler.
        Component.connect(self.stageEnd, "onTriggerEnter", self.entity, "onStageEndEnter")
    end
    log.info(string.format("[level_complete] scene=%s endX=%.1f", Scene.current() or "", self.endX))
end

function LevelComplete:onUpdate(entity, dt)
    if not dt or dt <= 0 then
        dt = 1.0 / 60.0
    end

    if self.completed then
        self:followCamera()
        self.timer = self.timer + dt
        if self.timer >= SHOW_TIME then
            Scene.load(NEXT_SCENE)
        end
        return
    end

    -- Camera-based completion (backup to the StageEnd trigger).
    if self.cameraEntity ~= nil then
        local pos = Transform.getPosition(self.cameraEntity)
        if pos.x >= self.endX then
            self:triggerComplete()
        end
    end
end

-- Called when something enters the StageEnd trigger. Only the player can
-- finish the stage this way.
function LevelComplete:onStageEndEnter(this, other)
    if self.completed then
        return
    end
    if Component.getColliderLayerName(other) == "Player" then
        self:triggerComplete()
    end
end

function LevelComplete:followCamera()
    if self.textEntity == nil or self.cameraEntity == nil then
        return
    end
    local cam = Transform.getPosition(self.cameraEntity)
    Transform.setPosition(self.textEntity, cam.x + TEXT_OFFSET_X, cam.y + TEXT_OFFSET_Y, 100)
end

function LevelComplete:triggerComplete()
    self.completed = true
    self.timer = 0.0

    self.textEntity = Entity.create()
    Entity.setName(self.textEntity, "StageClearText")
    Component.addText(self.textEntity, MESSAGE, FONT_SIZE, 255, 255, 255, 255)
    Component.setOpacity(self.textEntity, 1.0)
    self:followCamera()

    log.info("[level_complete] Level complete! Returning to level select")
end

return LevelComplete
