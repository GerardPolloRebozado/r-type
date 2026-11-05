-- Left of each block is draw() with no table, so the template defaults show.
-- Right passes only the fields to change. The script keeps that returned value.
local Button = UI.use("button")
local Checkbox = UI.use("checkbox")
local Combo = UI.use("combo")
local Confirm = UI.use("confirm")
local Image = UI.use("image")
local Input = UI.use("input")
local Menu = UI.use("menu")
local Progress = UI.use("progress")
local RadioGroup = UI.use("radio_group")
local Settings = UI.use("settings")
local Slider = UI.use("slider")
local SliderInt = UI.use("slider_int")
local Table = UI.use("table")

local Screen = {
    page = "catalog",
    sound = true,
    volume = 0.25,
    lives = 7,
    difficulty = 3,
    camera = 2,
    name = "R-9",
    lastChoice = "",
}

local function screenFlags()
    return ImGuiWindowFlags.NoDecoration
        | ImGuiWindowFlags.NoMove
        | ImGuiWindowFlags.NoSavedSettings
        | ImGuiWindowFlags.NoBringToFrontOnFocus
end

local function leaveOverlay(screen)
    local displayW, displayH = ImGui.GetDisplaySize()

    ImGui.SetNextWindowPos(displayW - 160, displayH - 64)
    ImGui.SetNextWindowSize(148, 52)
    ImGui.Begin(
        "##gallery-back",
        true,
        ImGuiWindowFlags.NoDecoration
            | ImGuiWindowFlags.NoMove
            | ImGuiWindowFlags.NoSavedSettings
            | ImGuiWindowFlags.NoNav
    )
    if Button.draw({ label = "Back", style = "ghost", width = 128, height = 32 }) then
        screen.page = "catalog"
    end
    ImGui.End()
end

local function beginPair(title)
    ImGui.Spacing()
    ImGui.Separator()
    ImGui.Text(title)
    ImGui.Columns(2, title, false)
    ImGui.TextDisabled("draw()")
end

local function changed(note)
    ImGui.NextColumn()
    ImGui.TextDisabled(note)
end

local function endPair()
    ImGui.Columns(1)
end

function Screen:onOpen(entity)
    self.page = "catalog"
    self.sound = true
    self.volume = 0.25
    self.lives = 7
    self.difficulty = 3
    self.camera = 2
    self.name = "R-9"
    self.lastChoice = ""
end

function Screen:draw(entity)
    local displayW = 0
    local displayH = 0
    local choice = nil
    local values = nil

    if self.page == "menu" then
        Menu.draw()
        leaveOverlay(self)
        return
    end

    if self.page == "menu_changed" then
        Menu.draw({
            message = "STAGE 1",
            items = {
                { label = "START", onClick = function() self.page = "catalog" end },
                { label = "BACK", onClick = function() self.page = "catalog" end },
            },
        })
        return
    end

    if self.page == "settings" then
        Settings.draw()
        leaveOverlay(self)
        return
    end

    if self.page == "settings_changed" then
        values = Settings.draw({
            title = "OPTIONS",
            rows = {
                { template = "slider", props = { label = "Volume", value = self.volume, min = 0, max = 1 } },
                { template = "checkbox", props = { label = "Sound", value = self.sound } },
                { template = "button", props = { label = "Back", style = "ghost", width = 140, height = 36 } },
            },
        })
        self.volume = values.Volume or self.volume
        if values.Sound ~= nil then
            self.sound = values.Sound
        end
        if values.Back then
            self.page = "catalog"
        end
        return
    end

    if self.page == "confirm" then
        choice = Confirm.draw()
        if choice then
            self.lastChoice = choice
            self.page = "catalog"
        end
        return
    end

    if self.page == "confirm_changed" then
        choice = Confirm.draw({
            title = "Leave",
            message = "Drop back to the list?",
            confirmLabel = "Yes",
            cancelLabel = "No",
        })
        if choice then
            self.lastChoice = choice
            self.page = "catalog"
        end
        return
    end

    displayW, displayH = ImGui.GetDisplaySize()
    ImGui.SetNextWindowPos(0, 0)
    ImGui.SetNextWindowSize(displayW, displayH)
    ImGui.Begin("##gallery", true, screenFlags())
    ImGui.Text("Templates")
    ImGui.TextDisabled("Left stays on the defaults, so a drag snaps back. Right keeps what you return.")
    ImGui.BeginChild("##list")

    beginPair("button")
    Button.draw()
    changed('label, style, width, height')
    Button.draw({ label = "Launch", style = "danger", width = 160, height = 36 })
    ImGui.SameLine()
    Button.draw({ label = "Ghost", style = "ghost", width = 110, height = 36 })
    ImGui.SameLine()
    Button.draw({
        label = "Yellow",
        color = { 1, 0.84, 0.16, 1 },
        text = { 0.1, 0.08, 0, 1 },
        width = 120,
        height = 36,
    })
    endPair()

    beginPair("checkbox")
    Checkbox.draw()
    changed("label, value")
    self.sound = Checkbox.draw({ label = "Sound", value = self.sound })
    endPair()

    beginPair("slider")
    Slider.draw()
    changed("label, value, min, max")
    self.volume = Slider.draw({ label = "Volume", value = self.volume, min = 0, max = 1 })
    endPair()

    beginPair("slider_int")
    SliderInt.draw()
    changed("label, value, min, max")
    self.lives = SliderInt.draw({ label = "Lives", value = self.lives, min = 1, max = 9 })
    endPair()

    beginPair("input")
    Input.draw()
    changed("label, value")
    self.name = Input.draw({ label = "Call sign", value = self.name })
    endPair()

    beginPair("combo")
    Combo.draw()
    changed("options, value. Indexes start at 1")
    self.difficulty = Combo.draw({
        label = "Difficulty",
        value = self.difficulty,
        options = { "Easy", "Normal", "Hard" },
    })
    endPair()

    beginPair("radio_group")
    RadioGroup.draw()
    changed("options, value. Indexes start at 1")
    self.camera = RadioGroup.draw({
        label = "Camera",
        value = self.camera,
        options = { "Ship", "Wide" },
    })
    endPair()

    beginPair("progress")
    Progress.draw()
    changed("value follows the Volume slider above")
    Progress.draw({ label = "Charge", value = self.volume })
    endPair()

    beginPair("table")
    Table.draw()
    ImGui.TextDisabled("No columns, so the default draws nothing.")
    changed("columns and rows. Cells start at 1")
    Table.draw({
        id = "scores",
        columns = { "Name", "Score" },
        rows = {
            { "Nico", "12000" },
            { self.name, "0" },
        },
    })
    endPair()

    beginPair("image")
    Image.draw()
    ImGui.TextDisabled("An empty path draws nothing.")
    changed("path, width, height")
    Image.draw({
        path = "../../R-Type/assets/player/sprite_000.png",
        width = 96,
        height = 48,
    })
    endPair()

    beginPair("menu, settings, confirm")
    ImGui.TextDisabled("These fill the screen. Back sits on the default pages.")
    if Button.draw({ label = "Menu", width = 160, height = 36 }) then
        self.page = "menu"
    end
    ImGui.SameLine()
    if Button.draw({ label = "Settings", width = 160, height = 36 }) then
        self.page = "settings"
    end
    ImGui.SameLine()
    if Button.draw({ label = "Confirm", width = 160, height = 36 }) then
        self.page = "confirm"
    end
    changed("message, items, title, labels")
    if Button.draw({ label = "Menu changed", style = "secondary", width = 180, height = 36 }) then
        self.page = "menu_changed"
    end
    ImGui.SameLine()
    if Button.draw({ label = "Settings changed", style = "secondary", width = 180, height = 36 }) then
        self.page = "settings_changed"
    end
    if Button.draw({ label = "Confirm changed", style = "secondary", width = 180, height = 36 }) then
        self.page = "confirm_changed"
    end
    if self.lastChoice ~= "" then
        ImGui.Text("Last confirm choice: " .. self.lastChoice)
    end
    endPair()

    ImGui.EndChild()
    ImGui.End()
end

return Screen
