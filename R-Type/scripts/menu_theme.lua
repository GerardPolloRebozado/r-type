-- menu_theme.lua
-- R-Type Menu Palette & Theme Controller
local MenuTheme = {}

-- Preset color palettes for R-Type
MenuTheme.Palettes = {
    -- Classic 1987 Arcade: Pitch black void, arcade gold, high-contrast dark button text
    ArcadeGold = {
        accent = { 1.0, 0.84, 0.16, 1.0 },
        panel = { 0.03, 0.03, 0.05, 0.95 },
        text = { 0.95, 0.94, 0.90, 1.0 },
        rounding = 3.0,
        fontScale = 1.15,
    },
    -- Wave Cannon: Deep abyss with electric neon cyan
    WaveCannon = {
        accent = { 0.15, 0.85, 1.0, 1.0 },
        panel = { 0.03, 0.04, 0.07, 1.0 },
        text = { 0.92, 0.96, 1.0, 1.0 },
        rounding = 3.0,
        fontScale = 1.15,
    },
    -- Bydo Empire: Crimson plasma & bio-mechanical violet
    Bydo = {
        accent = { 0.88, 0.18, 0.32, 1.0 },
        panel = { 0.05, 0.02, 0.07, 1.0 },
        text = { 0.94, 0.92, 0.86, 1.0 },
        rounding = 3.0,
        fontScale = 1.15,
    },
}

function MenuTheme:onCreate(entity)
    self.entity = entity
    local palette = self.Palettes.ArcadeGold

    UI.setTheme({
        accent = palette.accent,
        panel = palette.panel,
        text = palette.text,
        rounding = palette.rounding,
        fontScale = palette.fontScale,
    })

    log.info("[menu_theme.lua] Applied decorated R-Type color palette to menu.scene")
end

function MenuTheme:onUpdate(entity, dt)
    -- Optional runtime effects
end

return MenuTheme
