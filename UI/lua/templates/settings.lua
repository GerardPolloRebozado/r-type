-- Same black screen as the title menu. Children return by index, old rows by label.
-- These rows are always drawn above children and custom rows, and they are not returned:
--   VOL - / VOL + step each AudioSource by 0.1, clamped to 0..1, without matching them.
--   Mute music writes 0 on sources with isMusic and puts the saved volume back when cleared.
--   Mute effects does the same for sources with isMusic == false.
-- While a mute stays on, each draw keeps that category at 0. The first time a source
-- is seen its volume is saved, and that save is not replaced with the 0 just written.
-- No source in a category changes nothing. Editor preview draws the rows and does not
-- write component volume. Audio.playSound and Audio.playMusic are not AudioSource.
local Settings = {}

Settings.defaults = {
    title = "SETTINGS",
    rows = {},
}

Settings.musicMuted = false
Settings.effectsMuted = false
Settings.rememberedMusic = {}
Settings.rememberedEffects = {}

local STEP = 0.1
local RED = { 0.86, 0.08, 0.11, 1 }
local BONE = { 0.73, 0.66, 0.48, 1 }

local function screenFlags()
    return ImGuiWindowFlags.NoTitleBar
        | ImGuiWindowFlags.NoResize
        | ImGuiWindowFlags.NoMove
        | ImGuiWindowFlags.NoCollapse
        | ImGuiWindowFlags.NoSavedSettings
        | ImGuiWindowFlags.NoBringToFrontOnFocus
end

local function solidBar(id, width, height, color)
    UI.pushStyle({
        color = color,
        hover = color,
        active = color,
        rounding = 0,
    })
    ImGui.Button(id, width, height)
    UI.popStyle()
end

local function clamp01(value)
    if value < 0 then
        return 0
    end
    if value > 1 then
        return 1
    end
    return value
end

local function memoryFor(wantMusic)
    if wantMusic then
        return Settings.rememberedMusic
    end
    return Settings.rememberedEffects
end

local function writeVolume(entity, volume)
    if Component == nil or Component.setAudioVolume == nil then
        return
    end
    Component.setAudioVolume(entity, volume)
end

local function eachSource(callback)
    local entities = nil
    local source = nil

    if Component == nil or Component.entitiesWith == nil or Component.getAudioSource == nil then
        return
    end
    entities = Component.entitiesWith("AudioSource")
    if entities == nil then
        return
    end
    for _, entity in ipairs(entities) do
        source = Component.getAudioSource(entity)
        if source ~= nil then
            callback(entity, source)
        end
    end
end

local function isMusicSource(source)
    return source.isMusic and true or false
end

-- Remember the live volume once. Later draws must not store the 0 just written.
local function setCategoryMuted(wantMusic, muted)
    local memory = nil

    memory = memoryFor(wantMusic)
    eachSource(function(entity, source)
        local saved = 0

        if isMusicSource(source) ~= wantMusic then
            return
        end
        if muted then
            if memory[entity] == nil then
                memory[entity] = source.volume or 1
            end
            writeVolume(entity, 0)
            return
        end
        saved = memory[entity]
        if saved == nil then
            saved = source.volume or 1
        end
        memory[entity] = nil
        writeVolume(entity, saved)
    end)
end

local function holdMuted(wantMusic)
    local memory = nil

    memory = memoryFor(wantMusic)
    eachSource(function(entity, source)
        if isMusicSource(source) ~= wantMusic then
            return
        end
        if memory[entity] == nil then
            memory[entity] = source.volume or 1
        end
        if source.volume ~= 0 then
            writeVolume(entity, 0)
        end
    end)
end

local function stepSources(delta)
    eachSource(function(entity, source)
        local music = false
        local muted = false
        local memory = nil
        local current = 0
        local nextVolume = 0

        music = isMusicSource(source)
        muted = (music and Settings.musicMuted) or ((not music) and Settings.effectsMuted)
        memory = memoryFor(music)
        current = source.volume or 1
        if muted and memory[entity] ~= nil then
            current = memory[entity]
        end
        nextVolume = clamp01(current + delta)
        if muted then
            memory[entity] = nextVolume
            writeVolume(entity, 0)
            return
        end
        writeVolume(entity, nextVolume)
    end)
end

local function applyMuteFlag(wantMusic, muted)
    local previous = false

    if wantMusic then
        previous = Settings.musicMuted
    else
        previous = Settings.effectsMuted
    end
    muted = muted and true or false
    if wantMusic then
        Settings.musicMuted = muted
    else
        Settings.effectsMuted = muted
    end
    if muted ~= previous then
        setCategoryMuted(wantMusic, muted)
        return
    end
    if muted then
        holdMuted(wantMusic)
    end
end

local function drawAudioControls()
    local Button = UI.use("button")
    local Checkbox = UI.use("checkbox")
    local volumeDown = false
    local volumeUp = false
    local music = false
    local effects = false

    ImGui.PushID("settings-audio")
    ImGui.Text("Volume")
    volumeDown = Button.draw({ label = "VOL -", width = 140, height = 36 })
    ImGui.SameLine()
    volumeUp = Button.draw({ label = "VOL +", width = 140, height = 36 })
    music = Checkbox.draw({ label = "Mute music", value = Settings.musicMuted })
    effects = Checkbox.draw({ label = "Mute effects", value = Settings.effectsMuted })
    ImGui.PopID()
    ImGui.Spacing()

    if UI.isPreview() then
        Settings.musicMuted = music and true or false
        Settings.effectsMuted = effects and true or false
        return
    end
    if volumeDown then
        stepSources(-STEP)
    end
    if volumeUp then
        stepSources(STEP)
    end
    applyMuteFlag(true, music)
    applyMuteFlag(false, effects)
end

function Settings.draw(props)
    local p = UI.merge(Settings.defaults, props)
    local rows = p.rows or {}
    local children = p.children or {}
    local values = {}
    local displayW, displayH = UI.screenSize()
    local column = 520
    local title = p.title or "SETTINGS"
    local titleW = 0
    local index = 1
    local row = nil

    ImGui.PushStyleVar(ImGuiStyleVar.WindowPadding, 0, 0)
    ImGui.PushStyleVar(ImGuiStyleVar.WindowBorderSize, 0)
    ImGui.PushStyleColor(ImGuiCol.WindowBg, 0, 0, 0, 1)
    UI.beginScreen("##rtype-settings", screenFlags())

    ImGui.SetCursorPos(0, 0)
    solidBar("##settings-top-bone", displayW, 14, BONE)
    ImGui.SetCursorPos(0, 14)
    solidBar("##settings-top-red", displayW, 3, RED)

    ImGui.SetWindowFontScale(2.2)
    titleW = ImGui.CalcTextSize(title)
    ImGui.SetCursorPos((displayW - titleW) * 0.5, 36)
    ImGui.TextColored(RED[1], RED[2], RED[3], 1, title)
    ImGui.SetWindowFontScale(1)
    ImGui.SetCursorPos((displayW - 180) * 0.5, 84)
    solidBar("##settings-rule", 180, 3, RED)

    ImGui.SetCursorPos((displayW - column) * 0.5, 108)
    ImGui.BeginChild("##settings-rows", column, displayH - 150, ImGuiChildFlags.None)
    drawAudioControls()
    if #children > 0 then
        -- C++ puts a button child's onClick in its props, so button.lua already runs it.
        for index, row in ipairs(children) do
            local template = UI.use(row.template)
            ImGui.PushID("child" .. index)
            if template.draw then
                values[index] = template.draw(row.props or {})
            end
            ImGui.PopID()
            ImGui.Spacing()
        end
    else
        for index, row in ipairs(rows) do
            local template = UI.use(row.template or "label")
            local rowProps = row.props or {}
            local key = rowProps.label or row.template or index
            if template.draw then
                values[key] = template.draw(rowProps)
            end
            ImGui.Spacing()
        end
    end
    ImGui.EndChild()

    UI.endScreen()
    ImGui.PopStyleColor()
    ImGui.PopStyleVar(2)
    return values
end

return Settings
