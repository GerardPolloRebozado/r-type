-- waveSpawner.lua
-- Manages timed and camera-based enemy wave spawning using the engine Wave API.

local Script = {}

-- Waves schedule: each wave spawns when camera.x >= triggerX (or elapsed time)
local WAVES = {
    {
        name = "Wave 1 - Pata-Pata V-Formation",
        triggerX = -200.0,
        pattern = "vformation",
        prefabPath = "Prefabs/Pata-Pata.prefab",
        count = 5,
        spacing = 90.0,
        spawnOffsetX = 900.0,
        baseY = -120.0,
        attachParent = true,
        parentScriptPath = "scripts/group-patapata.lua",
        spawned = false
    },
    {
        name = "Wave 2 - Bink Ground Guard Line",
        triggerX = 300.0,
        pattern = "line",
        prefabPath = "Prefabs/Bink.prefab",
        count = 3,
        spacing = 140.0,
        spawnOffsetX = 1000.0,
        baseY = 232.0,
        attachParent = false,
        spawned = false
    },
    {
        name = "Wave 3 - Pata-Pata Sine Wave",
        triggerX = 1000.0,
        pattern = "sine",
        prefabPath = "Prefabs/Pata-Pata.prefab",
        count = 6,
        spacing = 80.0,
        spawnOffsetX = 900.0,
        baseY = 0.0,
        attachParent = true,
        parentScriptPath = "scripts/group-patapata.lua",
        spawned = false
    },
    {
        name = "Wave 4 - Bink Support Grid",
        triggerX = 2000.0,
        pattern = "grid",
        prefabPath = "Prefabs/Bink.prefab",
        count = 4,
        spacing = 120.0,
        spawnOffsetX = 950.0,
        baseY = 150.0,
        attachParent = false,
        spawned = false
    }
}

function Script:onCreate(entity)
    self.entity = entity
    self.elapsed = 0.0
    self.cameraEntity = nil

    -- Find camera entity
    self.cameraEntity = Entity.find("Camera")

    log.info("[WaveSpawner] Initialized. Camera entity: " .. tostring(self.cameraEntity))
end

function Script:onUpdate(entity, dt)
    if not dt or dt <= 0 then
        dt = 1.0 / 60.0
    end
    self.elapsed = self.elapsed + dt

    local camX = 0.0
    if self.cameraEntity ~= nil then
        local camPos = Transform.getPosition(self.cameraEntity)
        camX = camPos.x
    else
        -- Fallback to elapsed time if camera isn't found
        camX = -400.0 + self.elapsed * 60.0
    end

    for _, wave in ipairs(WAVES) do
        if not wave.spawned and camX >= wave.triggerX then
            wave.spawned = true
            log.info(string.format("[WaveSpawner] Spawning %s at camX=%.1f", wave.name, camX))

            local entities = Wave.spawnFormation({
                prefabPath = wave.prefabPath,
                pattern = wave.pattern,
                count = wave.count,
                spacing = wave.spacing,
                attachParent = wave.attachParent or false,
                parentScriptPath = wave.parentScriptPath or ""
            })

            -- Shift wave formation relative to camera and target baseY
            local startX = camX + wave.spawnOffsetX
            for i, childId in ipairs(entities) do
                local p = Transform.getPosition(childId)
                Transform.setPosition(childId, startX + p.x, wave.baseY + p.y)
            end

            -- If attached to a parent group, position parent entity as well
            if wave.attachParent and #entities > 0 then
                local parentId = Component.getParent(entities[1])
                if parentId then
                    Transform.setPosition(parentId, startX, wave.baseY)
                end
            end
        end
    end
end

return Script
