-- server.lua
-- Attached to: Server

local Script = {
    isServer = false
}

function Script:onCreate(entity)
    self.entity = entity
    log.info("[" .. tostring(entity) .. "] Script created")
    if (arg[1] and arg[1] == "--server") then
        print("Running as server")
        isServer = true
    end
end

function Script:onUpdate(entity, dt)
    if isServer ~= true then
        return
    end
end

function Script:onDestroy(entity)
    log.info("[" .. tostring(entity) .. "] Script destroyed")
end

return Script
