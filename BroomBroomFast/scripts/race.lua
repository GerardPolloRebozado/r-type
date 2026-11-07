-- race.lua
-- Runs one race: car colours, countdown, laps and positions, car-to-car
-- bumps, camera, HUD and the results screen.
-- Shares its state with player.lua and ai_car.lua through the RaceState global.

local Keys = require("scripts.keys")
local GameState = require("scripts.game_state")

local Race = {}

local LAPS = 3
local CELL = 256 -- size of one track cell in the scenes
local WAYPOINT_RADIUS = 230
local COUNTDOWN = 3.0
local CAR_RADIUS = 24 -- cars closer than 2 * CAR_RADIUS get pushed apart
local VIEW_W, VIEW_H = 1920, 1080
local CAMERA_SPEED = 6.0
local YELLOW = { 255, 210, 60 }
local SHAPE_RECTANGLE = 0 -- ShapeType::RECTANGLE

local function cellKey(x, y)
    return math.floor(x / CELL) .. ":" .. math.floor(y / CELL)
end

local function ordinal(n)
    return ({ "1st", "2nd", "3rd" })[n] or (n .. "th")
end

local function formatTime(t)
    return string.format("%d:%04.1f", math.floor(t / 60), t % 60)
end

local function shuffle(list)
    for i = #list, 2, -1 do
        local j = math.random(i)
        list[i], list[j] = list[j], list[i]
    end
end

-- Only rewrites a text when it changed.
function Race:setText(name, value, size, color)
    local entity = self.hud[name]
    if not entity then
        return
    end
    local key = value .. "|" .. size
    if self.hudCache[name] ~= key then
        local c = color or { 255, 255, 255 }
        Component.addText(entity, value, size, c[1], c[2], c[3], 255)
        self.hudCache[name] = key
    end
end

function Race:onCreate(entity)
    self.keys = Keys.new()
    self.leaving = false
    self.hud = {}
    self.hudCache = {}
    for _, name in ipairs({ "HudPanel", "HudLap", "HudPos", "HudTime", "HudBig", "HudBigPanel", "HudHint" }) do
        self.hud[name] = Entity.find(name)
    end
    self.camera = Entity.find("Camera")

    local rs = {
        ready = false,
        phase = "countdown", -- countdown -> race -> finished
        timer = COUNTDOWN,
        time = 0.0,
        laps = LAPS,
        cars = {},
        waypoints = {},
        road = {},
        finishedCount = 0,
        width = 3584,
        height = 2304,
    }

    -- Track cells: every tile named "Road".
    for _, e in ipairs(Component.entitiesWith("Sprite")) do
        if Entity.getName(e) == "Road" then
            local p = Transform.getPosition(e)
            rs.road[cellKey(p.x + 1, p.y + 1)] = true
        end
    end

    local i = 1
    while true do
        local wp = Entity.find("Waypoint" .. i)
        if not wp then
            break
        end
        local p = Transform.getPosition(wp)
        rs.waypoints[i] = { x = p.x, y = p.y }
        i = i + 1
    end

    local bounds = Entity.find("MapBounds")
    if bounds then
        local p = Transform.getPosition(bounds)
        rs.width, rs.height = p.x, p.y
    end

    function rs.isOnRoad(x, y)
        return rs.road[cellKey(x, y)] == true
    end

    function rs.clamp(car)
        car.x = math.max(20, math.min(rs.width - 20, car.x))
        car.y = math.max(20, math.min(rs.height - 20, car.y))
    end

    -- Player gets the chosen colour, the AI cars get three of the others.
    local others = {}
    for _, color in ipairs(GameState.CARS) do
        if color ~= GameState.car then
            table.insert(others, color)
        end
    end
    shuffle(others)
    local player = Entity.find("Player")
    if player then
        Component.setSpritePath(player, GameState.carSprite(GameState.car))
    end
    for n = 1, 3 do
        local ai = Entity.find("AI" .. n)
        if ai then
            Component.setSpritePath(ai, GameState.carSprite(others[n]))
        end
    end

    rs.ready = #rs.waypoints > 0
    if not rs.ready then
        log.error("[race.lua] no waypoints found in " .. Scene.current())
    end
    RaceState = rs
end

function Race:onDestroy(entity)
    RaceState = nil
end

-- Moves cars to their next waypoint and counts laps.
function Race:updateProgress(rs)
    local count = #rs.waypoints
    for _, car in ipairs(rs.cars) do
        local wp = rs.waypoints[car.nextWaypoint]
        local dx, dy = wp.x - car.x, wp.y - car.y
        if dx * dx + dy * dy < WAYPOINT_RADIUS * WAYPOINT_RADIUS then
            car.nextWaypoint = car.nextWaypoint + 1
            if car.nextWaypoint > count then
                car.nextWaypoint = 1
                car.lap = car.lap + 1
                if car.lap >= rs.laps and not car.finished then
                    car.finished = true
                    rs.finishedCount = rs.finishedCount + 1
                    car.place = rs.finishedCount
                    car.finishTime = rs.time
                end
            end
        end
    end
end

function Race:score(rs, car)
    if car.finished then
        return 1e9 - car.place
    end
    local count = #rs.waypoints
    local wp = rs.waypoints[car.nextWaypoint]
    local d = math.sqrt((wp.x - car.x) ^ 2 + (wp.y - car.y) ^ 2)
    local closeness = 1.0 - math.min(1.0, d / (CELL * 2))
    return car.lap * count + (car.nextWaypoint - 1) + closeness * 0.99
end

function Race:playerPosition(rs)
    local player = rs.player
    if not player then
        return #rs.cars
    end
    local mine = self:score(rs, player)
    local pos = 1
    for _, car in ipairs(rs.cars) do
        if car ~= player and self:score(rs, car) > mine then
            pos = pos + 1
        end
    end
    return pos
end

-- Pushes overlapping cars apart and slows them a little.
function Race:bumpCars(rs)
    local minDist = CAR_RADIUS * 2
    for i = 1, #rs.cars do
        for j = i + 1, #rs.cars do
            local a, b = rs.cars[i], rs.cars[j]
            local dx, dy = b.x - a.x, b.y - a.y
            local d = math.sqrt(dx * dx + dy * dy)
            if d < minDist then
                if d < 0.001 then
                    dx, dy, d = 1, 0, 1
                end
                local push = (minDist - d) / 2
                local nx, ny = dx / d, dy / d
                a.x, a.y = a.x - nx * push, a.y - ny * push
                b.x, b.y = b.x + nx * push, b.y + ny * push
                a.speed = a.speed * 0.97
                b.speed = b.speed * 0.97
            end
        end
    end
end

function Race:updateCamera(rs, dt)
    if not self.camera or not rs.player then
        return 0, 0
    end
    local p = Transform.getPosition(self.camera)
    local t = math.min(1.0, CAMERA_SPEED * dt)
    local x = p.x + (rs.player.x - p.x) * t
    local y = p.y + (rs.player.y - p.y) * t
    x = math.max(VIEW_W / 2, math.min(rs.width - VIEW_W / 2, x))
    y = math.max(VIEW_H / 2, math.min(rs.height - VIEW_H / 2, y))
    Transform.setPosition(self.camera, x, y, 0)
    return x, y
end

function Race:placeHud(name, x, y, z)
    local e = self.hud[name]
    if e then
        Transform.setPosition(e, x, y, z)
    end
end

function Race:updateHud(rs, camX, camY)
    local left, top = camX - VIEW_W / 2, camY - VIEW_H / 2
    local player = rs.player

    self:placeHud("HudPanel", left + 20, top + 20, 99)
    self:placeHud("HudLap", left + 45, top + 35, 100)
    self:placeHud("HudPos", left + 45, top + 95, 100)
    self:placeHud("HudTime", left + 45, top + 155, 100)

    local lap = player and math.min(player.lap + 1, rs.laps) or 1
    self:setText("HudLap", "LAP " .. lap .. "/" .. rs.laps, 50)
    self:setText("HudPos", "POS " .. self:playerPosition(rs) .. "/" .. #rs.cars, 50)
    local shownTime = (player and player.finishTime) or rs.time
    self:setText("HudTime", "TIME " .. formatTime(math.floor(shownTime * 10) / 10), 40)

    -- Big centre message: countdown, GO!, results.
    local big, bigSize, hint = "", 120, ""
    if rs.phase == "countdown" then
        big = tostring(math.ceil(rs.timer))
    elseif rs.phase == "race" and rs.time < 1.0 then
        big = "GO!"
    elseif rs.phase == "finished" and player then
        big = ordinal(player.place) .. " PLACE!"
        hint = "Time " .. formatTime(player.finishTime) .. "   -   Press ENTER to go back to the menu"
    end
    self:setText("HudBig", big, bigSize, YELLOW)
    self:setText("HudHint", hint, 36)

    local bigWidth = #big * bigSize * 0.62
    local hintWidth = #hint * 36 * 0.62
    self:placeHud("HudBig", camX - bigWidth / 2, camY - 200, 100)
    self:placeHud("HudHint", camX - hintWidth / 2, camY - 50, 100)
    if big ~= "" then
        local panelW = math.floor(math.max(bigWidth, hintWidth) + 80)
        local panelH = hint ~= "" and 240 or 170
        self:placeHud("HudBigPanel", camX - panelW / 2, camY - 230, 99)
        local size = panelW .. "x" .. panelH
        if self.hud.HudBigPanel and self.panelSize ~= size then
            Component.addShape(self.hud.HudBigPanel, SHAPE_RECTANGLE, panelW, panelH,
                0, 3, 1, true, 20, 20, 30, 170)
            self.panelSize = size
        end
    else
        self:placeHud("HudBigPanel", -100000, -100000, 99)
    end
end

function Race:onUpdate(entity, dt)
    self.keys:update()
    local rs = RaceState
    if not rs or not rs.ready then
        return
    end

    if rs.phase == "countdown" then
        rs.timer = rs.timer - dt
        if rs.timer <= 0 then
            rs.phase = "race"
        end
    else
        rs.time = rs.time + dt
        self:updateProgress(rs)
        self:bumpCars(rs)
        if rs.phase == "race" and rs.player and rs.player.finished then
            rs.phase = "finished"
        end
    end

    local camX, camY = self:updateCamera(rs, dt)
    self:updateHud(rs, camX, camY)

    if self.leaving then
        return
    end
    if self.keys:pressed("back") or (rs.phase == "finished" and self.keys:pressed("accept")) then
        self.leaving = true
        Scene.load("Menu.scene")
    end
end

return Race
