-- Same black screen as the title menu. Children return by index, old rows by label.
local Settings = {}

Settings.defaults = {
    title = "SETTINGS",
    rows = {},
}

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
