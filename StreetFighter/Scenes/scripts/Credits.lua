-- Credits screen: Enter goes back to the country menu.
-- The menu music (Stage Select) keeps playing, the menu does not restart it.
local Script = {}

local MENU_SCENE = "game_menu.scene"

function Script:onCreate(entity)
    -- Enter is still down from choosing "Credits": wait for it to be released
    self.confirmHeld = Controller.isActionActive("Intro", 1)
    self.done = false
end

function Script:onUpdate(entity, dt)
    if self.done then return end
    local confirm = Controller.isActionActive("Intro", 1)
    if confirm and not self.confirmHeld then
        self.done = true
        Scene.load(MENU_SCENE)
    end
    self.confirmHeld = confirm
end

return Script
