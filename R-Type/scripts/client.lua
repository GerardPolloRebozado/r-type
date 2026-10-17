-- client.lua
-- Attached to: Client

local Script = {

    isClient = false
}

function Script:onCreate(entity)
    self.entity = entity
    log.info("[" .. tostring(entity) .. "] Script created")
    if (arg[1] and arg[1] ~= "--server") then
        print("Running as client")
        isClient = true
    end
end

function Script:onUpdate(entity, dt)
    if isClient ~= true then
        return
    end
    -- Add your per-frame logic here
end

function Script:onDestroy(entity)
    log.info("[" .. tostring(entity) .. "] Script destroyed")
end

return Script
