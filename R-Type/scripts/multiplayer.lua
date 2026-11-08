-- multiplayer.lua
-- Handles multiplayer connection and game start synchronization.
--
-- Attached to: MultiplayerManager, Play, HOST, CONNECT, MPHostInput, MPPortInput entities
--
-- Shared state lives in _G so all script instances see the same values.
-- Host/port come from --host/--port launch args (env vars RP_TYPE_HOST/RP_TYPE_PORT)
-- or defaults. Input fields display the current values (read-only).
-- Net.on handlers are registered once via a _G guard.

local Multiplayer = {}

-- Shared configuration (from --host/--port env vars or defaults)
if _G.MP_HOST == nil then
    _G.MP_HOST = os.getenv("RP_TYPE_HOST") or "127.0.0.1"
end
if _G.MP_PORT == nil then
    _G.MP_PORT = tonumber(os.getenv("RP_TYPE_PORT")) or 3000
end
if _G.MP_CONNECTED == nil then _G.MP_CONNECTED = false end
if _G.MP_IS_SERVER == nil then _G.MP_IS_SERVER = false end
if _G.MP_NET_HANDLER == nil then _G.MP_NET_HANDLER = false end
if _G.MP_CONN_HANDLER == nil then _G.MP_CONN_HANDLER = false end

-- Helper: update the status text entity
local function setStatus(text)
    local statusEnt = Entity.find("MPStatus")
    if statusEnt ~= nil then
        Component.setText(statusEnt, text)
    end
    log.info("[Multiplayer] " .. text)
end

function Multiplayer:onCreate(entity)
    self.entity = entity
    Entity.setName(entity, "MultiplayerManager")

    -- Register the start_game packet handler exactly once.
    -- Server broadcasts it; clients receive it and load the level selector.
    if Net ~= nil and Net.on ~= nil and not _G.MP_NET_HANDLER then
        _G.MP_NET_HANDLER = true
        Net.on("start_game", function(peer, data)
            if Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked() and not Net.isServer() then
                log.info("[Multiplayer] Received START_GAME, loading level selector")
                Scene.load("level_selector.scene")
            end
        end)
    end

    -- Handle connect/disconnect events for status updates
    if Net ~= nil and Net.onConnect ~= nil and not _G.MP_CONN_HANDLER then
        _G.MP_CONN_HANDLER = true
        Net.onConnect(function(peer)
            if Net.isServer() then
                setStatus("Status: Client connected")
            else
                setStatus("Status: Connected to server")
                _G.MP_CONNECTED = true
            end
        end)
    end

    -- Reflect already-networked state (from --server/--client launch args)
    if Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked() then
        _G.MP_CONNECTED = true
        _G.MP_IS_SERVER = Net.isServer()
        if _G.MP_IS_SERVER then
            setStatus("Status: Server on port " .. _G.MP_PORT)
        else
            setStatus("Status: Connected to " .. _G.MP_HOST .. ":" .. _G.MP_PORT)
        end
    else
        setStatus("Status: Disconnected")
    end
end

-- Called by UI button: start as server (host)
function Multiplayer:connectServer()
    if Net == nil or Net.startServer == nil then
        log.error("[Multiplayer] Net API not available")
        setStatus("Status: Net API not available")
        return
    end
    if _G.MP_CONNECTED then
        log.info("[Multiplayer] Already networked")
        return
    end
    local ok = Net.startServer(_G.MP_PORT, 4)
    if ok then
        _G.MP_CONNECTED = true
        _G.MP_IS_SERVER = true
        setStatus("Status: Server on port " .. _G.MP_PORT)
        log.info("[Multiplayer] Server listening on port " .. _G.MP_PORT)
    else
        setStatus("Status: Server start failed")
        log.error("[Multiplayer] Could not start server on port " .. _G.MP_PORT)
    end
end

-- Called by UI button: connect as client
function Multiplayer:connectClient()
    if Net == nil or Net.startClient == nil then
        log.error("[Multiplayer] Net API not available")
        setStatus("Status: Net API not available")
        return
    end
    if _G.MP_CONNECTED then
        log.info("[Multiplayer] Already networked")
        return
    end
    local ok = Net.startClient(_G.MP_HOST, _G.MP_PORT)
    if ok then
        setStatus("Status: Connecting to " .. _G.MP_HOST .. ":" .. _G.MP_PORT)
        log.info("[Multiplayer] Connecting to " .. _G.MP_HOST .. ":" .. _G.MP_PORT)
    else
        setStatus("Status: Connect failed")
        log.error("[Multiplayer] Could not connect to " .. _G.MP_HOST .. ":" .. _G.MP_PORT)
    end
end

-- Called by UI button: disconnect
function Multiplayer:disconnect()
    _G.MP_CONNECTED = false
    _G.MP_IS_SERVER = false
    setStatus("Status: Disconnected")
    log.info("[Multiplayer] Disconnected")
end

-- Called when Play is clicked from main menu.
function Multiplayer:onPlayClicked()
    if Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked() then
        if Net.isServer() then
            Net.broadcast("start_game", {}, true, 0)
            log.info("[Multiplayer] Broadcasting START_GAME to all clients")
            Scene.load("level_selector.scene")
        end
        -- Client: the server's start_game packet will trigger Scene.load
    else
        Scene.load("level_selector.scene")
    end
end

function Multiplayer:onUpdate(entity, dt)
end

return Multiplayer
