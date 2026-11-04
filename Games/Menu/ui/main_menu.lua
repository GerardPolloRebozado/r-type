-- Pages live on this table. The component reloads the file; there is no screen stack.
local Menu = UI.use("menu")
local Settings = UI.use("settings")

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
    UI.setTheme({
        accent = { 1.0, 0.84, 0.16, 1 },
        panel = { 0, 0, 0, 1 },
        text = { 0.95, 0.94, 0.90, 1 },
        rounding = 0,
        fontScale = 1,
    })
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
        -- An unchecked box returns false, which is not the same as a missing key.
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
        Menu.draw({
            message = "LEAVE THE GAME?",
            items = {
                { label = "QUIT", onClick = function() self.page = "playing" end },
                { label = "STAY", onClick = function() self.page = "menu" end },
            },
        })
        return
    end

    if self.page == "playing" then
        Menu.draw({
            message = "R-9  LAUNCHED",
            items = {
                { label = "BACK", onClick = function() self.page = "menu" end },
            },
        })
        return
    end

    Menu.draw({
        items = {
            { label = "PLAY", onClick = function() self.page = "playing" end },
            { label = "SETTINGS", onClick = function() self.page = "settings" end },
            { label = "QUIT", onClick = function() self.page = "confirm" end },
        },
    })
end

return Screen
