-- keys.lua
-- Menu key presses. A key counts once per press, even when the engine runs
-- several updates in one frame or a key is still held from the previous scene.

local Keys = {}
Keys.__index = Keys

local ACTIONS = {
    up = { KEY.UP, KEY.W },
    down = { KEY.DOWN, KEY.S },
    left = { KEY.LEFT, KEY.A },
    right = { KEY.RIGHT, KEY.D },
    accept = { KEY.ENTER, KEY.SPACE },
    back = { KEY.ESCAPE, KEY.BACKSPACE },
}

local function isDown(action)
    for _, key in ipairs(ACTIONS[action]) do
        if Input.isKeyDown(key) then
            return true
        end
    end
    return false
end

function Keys.new()
    local self = setmetatable({ now = {}, before = {} }, Keys)
    -- Keys held while the scene starts are ignored until released.
    for action in pairs(ACTIONS) do
        self.now[action] = isDown(action)
    end
    return self
end

-- Call once at the start of every onUpdate.
function Keys:update()
    for action in pairs(ACTIONS) do
        self.before[action] = self.now[action]
        self.now[action] = isDown(action)
    end
end

function Keys:pressed(action)
    return self.now[action] and not self.before[action]
end

return Keys
