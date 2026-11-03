local Menu = UI.use("menu")
local Settings = UI.use("settings")
local Confirm = UI.use("confirm")
local Progress = UI.use("progress")

local Screen = {
    page = "menu",
    volume = 0.8,
    fullscreen = false,
    difficulty = 2,
    name = "Pilot",
}

function Screen:onOpen(entity)
    self.page = "menu"
    self.volume = 0.8
    self.fullscreen = false
    self.difficulty = 2
    self.name = "Pilot"
end

function Screen:draw(entity)
    if self.page == "settings" then
        local values = Settings.draw({
            title = "Settings",
            rows = {
                { template = "slider", props = { label = "Volume", value = self.volume, min = 0, max = 1 } },
                { template = "checkbox", props = { label = "Fullscreen", value = self.fullscreen } },
                {
                    template = "combo",
                    props = {
                        label = "Difficulty",
                        value = self.difficulty,
                        options = { "Easy", "Normal", "Hard" },
                    },
                },
                { template = "input", props = { label = "Name", value = self.name } },
                { template = "slider_int", props = { label = "Lives", value = self.lives or 3, min = 1, max = 9 } },
                {
                    template = "radio_group",
                    props = {
                        label = "Camera",
                        value = self.camera or 1,
                        options = { "Ship", "Wide" },
                    },
                },
                { template = "progress", props = { label = "Charge", value = self.volume } },
                {
                    template = "table",
                    props = {
                        id = "scores",
                        columns = { "Name", "Score" },
                        rows = {
                            { "Nico", "12000" },
                            { "Ada", "8000" },
                            { self.name, "0" },
                        },
                    },
                },
                { template = "button", props = { label = "Back", style = "ghost", width = 140, height = 36 } },
            },
        })
        self.volume = values.Volume or self.volume
        if values.Fullscreen ~= nil then
            self.fullscreen = values.Fullscreen
        end
        self.difficulty = values.Difficulty or self.difficulty
        self.name = values.Name or self.name
        self.lives = values.Lives or self.lives
        self.camera = values.Camera or self.camera
        if values.Back then
            self.page = "menu"
        end
        return
    end

    if self.page == "confirm" then
        local choice = Confirm.draw({
            title = "Quit",
            message = "Leave the menu demo?",
            confirmLabel = "Quit",
            cancelLabel = "Stay",
        })
        if choice == "confirm" then
            self.page = "playing"
        elseif choice == "cancel" then
            self.page = "menu"
        end
        return
    end

    if self.page == "playing" then
        local displayW, displayH = ImGui.GetDisplaySize()
        ImGui.SetNextWindowPos((displayW - 360) * 0.5, (displayH - 180) * 0.5)
        ImGui.SetNextWindowSize(360, 180)
        ImGui.Begin("Playing")
        Progress.draw({ label = "In game", value = self.volume })
        if ImGui.Button("Back to menu", 180, 36) then
            self.page = "menu"
        end
        ImGui.End()
        return
    end

    Menu.draw({
        title = "R-TYPE",
        items = {
            { label = "Play", style = "primary", onClick = function() self.page = "playing" end },
            { label = "Settings", style = "secondary", onClick = function() self.page = "settings" end },
            { label = "Quit", style = "danger", onClick = function() self.page = "confirm" end },
        },
    })
end

return Screen
