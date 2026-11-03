local Menu = {}

Menu.defaults = {
    title = "R-TYPE",
    items = {},
    message = "",
}

local RED = { 0.86, 0.08, 0.11, 1 }
local WHITE = { 0.95, 0.94, 0.90, 1 }
local YELLOW = { 1.0, 0.84, 0.16, 1 }
local BONE = { 0.73, 0.66, 0.48, 1 }
local BLUE = { 0.10, 0.28, 0.62, 1 }

local function screenFlags()
    return ImGuiWindowFlags.NoDecoration
        | ImGuiWindowFlags.NoMove
        | ImGuiWindowFlags.NoSavedSettings
        | ImGuiWindowFlags.NoNav
        | ImGuiWindowFlags.NoBringToFrontOnFocus
end

local function beginScreen(id)
    local displayW, displayH = ImGui.GetDisplaySize()

    ImGui.SetNextWindowPos(0, 0)
    ImGui.SetNextWindowSize(displayW, displayH)
    ImGui.PushStyleVar(ImGuiStyleVar.WindowPadding, 0, 0)
    ImGui.PushStyleVar(ImGuiStyleVar.WindowBorderSize, 0)
    ImGui.PushStyleColor(ImGuiCol.WindowBg, 0, 0, 0, 1)
    ImGui.Begin(id, true, screenFlags())
    return displayW, displayH
end

local function endScreen()
    ImGui.End()
    ImGui.PopStyleColor()
    ImGui.PopStyleVar(2)
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

local function scoreLine(windowW)
    local hiW = 0

    ImGui.SetCursorPos(36, 28)
    ImGui.TextColored(RED[1], RED[2], RED[3], 1, "1UP")
    ImGui.SameLine()
    ImGui.TextColored(YELLOW[1], YELLOW[2], YELLOW[3], 1, "000000")

    hiW = ImGui.CalcTextSize("HI   100000")
    ImGui.SetCursorPos((windowW - hiW) * 0.5, 28)
    ImGui.TextColored(RED[1], RED[2], RED[3], 1, "HI")
    ImGui.SameLine()
    ImGui.TextColored(YELLOW[1], YELLOW[2], YELLOW[3], 1, "100000")

    ImGui.SetCursorPos(windowW - 150, 28)
    ImGui.TextColored(RED[1], RED[2], RED[3], 1, "2UP")
    ImGui.SameLine()
    ImGui.TextColored(YELLOW[1], YELLOW[2], YELLOW[3], 1, "000000")
end

local function logo(windowW, top)
    local text = "R-TYPE"
    local widths = {}
    local total = 0
    local x = 0
    local shadow = 0
    local index = 1
    local ch = ""

    ImGui.SetWindowFontScale(3.6)
    for index = 1, #text do
        ch = text:sub(index, index)
        widths[index] = ImGui.CalcTextSize(ch)
        total = total + widths[index]
    end
    x = (windowW - total) * 0.5
    shadow = x + 5
    for index = 1, #text do
        ImGui.SetCursorPos(shadow, top + 5)
        ImGui.TextColored(BLUE[1], BLUE[2], BLUE[3], 1, text:sub(index, index))
        shadow = shadow + widths[index]
    end
    for index = 1, #text do
        ch = text:sub(index, index)
        ImGui.SetCursorPos(x, top)
        if ch == "R" then
            ImGui.TextColored(RED[1], RED[2], RED[3], 1, ch)
        else
            ImGui.TextColored(WHITE[1], WHITE[2], WHITE[3], 1, ch)
        end
        x = x + widths[index]
    end
    ImGui.SetWindowFontScale(1)
    ImGui.SetCursorPos((windowW - 240) * 0.5, top + 78)
    solidBar("##logo-rule", 240, 3, RED)
end

local function itemRow(label, index, active, blink, windowW, y)
    local rowW = 320
    local x = (windowW - rowW) * 0.5
    local clicked = false
    local mark = " "

    ImGui.SetCursorPos(x, y)
    ImGui.PushStyleVar(ImGuiStyleVar.FrameBorderSize, 0)
    UI.pushStyle({
        color = { 0, 0, 0, 0 },
        hover = { 0.86, 0.08, 0.11, 0.22 },
        active = { 0.86, 0.08, 0.11, 0.35 },
        text = { 0, 0, 0, 0 },
        rounding = 0,
    })
    clicked = ImGui.Button("##" .. label, rowW, 42)
    UI.popStyle()
    ImGui.PopStyleVar()
    if ImGui.IsItemHovered() then
        Menu.cursor = index
    end

    ImGui.SetWindowFontScale(1.55)
    ImGui.SetCursorPos(x + 18, y + 6)
    if active and blink then
        mark = ">"
    end
    ImGui.TextColored(RED[1], RED[2], RED[3], 1, mark)
    ImGui.SameLine()
    if active then
        ImGui.TextColored(YELLOW[1], YELLOW[2], YELLOW[3], 1, label)
    else
        ImGui.TextColored(WHITE[1], WHITE[2], WHITE[3], 1, label)
    end
    ImGui.SetWindowFontScale(1)
    return clicked
end

function Menu.draw(props)
    local p = UI.merge(Menu.defaults, props)
    local items = p.items or {}
    local windowW = 0
    local windowH = 0
    local blink = false
    local y = 0
    local index = 1
    local item = nil

    if Menu.count ~= #items then
        Menu.cursor = 1
        Menu.count = #items
    end
    if not Menu.cursor or Menu.cursor < 1 then
        Menu.cursor = 1
    end
    blink = (ImGui.GetTime() % 0.66) < 0.33

    if #items > 0 and ImGui.IsKeyPressed(ImGuiKey.DownArrow, true) then
        Menu.cursor = (Menu.cursor % #items) + 1
    end
    if #items > 0 and ImGui.IsKeyPressed(ImGuiKey.UpArrow, true) then
        Menu.cursor = ((Menu.cursor - 2) % #items) + 1
    end

    windowW, windowH = beginScreen("##rtype-menu")
    ImGui.SetCursorPos(0, 0)
    solidBar("##top-bone", windowW, 14, BONE)
    ImGui.SetCursorPos(0, 14)
    solidBar("##top-red", windowW, 3, RED)
    scoreLine(windowW)
    logo(windowW, windowH * 0.20)

    if p.message and p.message ~= "" then
        local messageW = 0
        ImGui.SetWindowFontScale(1.2)
        messageW = ImGui.CalcTextSize(p.message)
        ImGui.SetCursorPos((windowW - messageW) * 0.5, windowH * 0.20 + 100)
        ImGui.TextColored(BONE[1], BONE[2], BONE[3], 1, p.message)
        ImGui.SetWindowFontScale(1)
    end

    y = windowH * 0.52
    for index, item in ipairs(items) do
        local label = item.label or "ITEM"
        local clicked = itemRow(label, index, index == Menu.cursor, blink, windowW, y)
        if clicked or (index == Menu.cursor and ImGui.IsKeyPressed(ImGuiKey.Enter)) then
            if type(item.onClick) == "function" then
                item.onClick()
            end
        end
        y = y + 50
    end

    ImGui.SetCursorPos(0, windowH - 17)
    solidBar("##bottom-red", windowW, 3, RED)
    ImGui.SetCursorPos(0, windowH - 14)
    solidBar("##bottom-bone", windowW, 14, BONE)
    endScreen()
end

return Menu
