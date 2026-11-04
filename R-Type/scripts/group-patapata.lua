-- group-patapata.lua
-- Attached to: Grouped Pata-Pata
--

local DRIFT_SPEED = 130.0 -- pixels per second, leftward
local TRI_AMP = 80.0 -- vertical zigzag half-height in pixels
local TRI_FREQ = 0.9 -- zigzag cycles per second

local TRAIL_KEEP_PX = 1500.0

local SPACING = 100.0 -- pitch between links; must match enemy1.lua (and the authored layout)

local Script = {}

-- Link order along the trail: entity ids say nothing about where a link sits in
-- the formation (getChildren order is arbitrary, and pasting one copies ids in
-- creation order), so sort by x - left of the drift means "in front".
local function SortByX(ids)
    table.sort(ids, function(a, b)
        return Transform.getPosition(a).x < Transform.getPosition(b).x
    end)
    return ids
end

-- Lays a straight trail behind the (invisible) leader.
--
-- The links sample the trail at exact i*SPACING steps behind the leader, so the
-- leader must sit *ahead* of every link. The scene may place the group node
-- anywhere in the formation though, and a link authored in front of the leader
-- folds the seeded polyline back on itself: every sample past the fold then
-- lands on the wrong side, so the links swap places and pile up on each other
-- until the fold ages out of the trail (~12s at DRIFT_SPEED).
-- Re-anchor the leader at the head instead of trusting the authored layout: it
-- holds no sprite, and the links are driven from the trail every frame anyway.
local function SeedTrail(entity)
    _PataTrails = _PataTrails or {}
    local key = "trail_" .. tostring(entity)
    local trail = {}
    _PataTrails[key] = trail

    local sibs = Component.getChildren(entity) or {}
    SortByX(sibs)

    -- Drift is leftward, so the formation trails to the right (+x): the head
    -- sits one SPACING step left of the leading link, at the links' height.
    local head = Transform.getPosition(entity)
    if #sibs > 0 then
        local lead = Transform.getPosition(sibs[1])
        head = { x = lead.x - SPACING, y = lead.y }
        Transform.setPosition(entity, head.x, head.y)
    end

    table.insert(trail, { x = head.x, y = head.y })
    for step = 1, #sibs + 2 do
        table.insert(trail, { x = head.x + SPACING * step, y = head.y })
    end
end

function Script:onCreate(entity)
    self.entity = entity
    self.time = 0.5 / TRI_FREQ
    if Net ~= nil then
        if Net.isNetworked() and Net.isServer() == false then
            return
        end
    end
    SeedTrail(entity)
    -- Read after seeding: SeedTrail may re-anchor the leader at the head.
    self.baseY = Transform.getPosition(entity).y
    log.info("[" .. tostring(entity) .. "] Script created")
end

function Script:onUpdate(entity, dt)
    if Net ~= nil then
        if Net.isNetworked() and Net.isServer() == false then
            return
        end
    end

    if not dt or dt <= 0 then
        dt = 1.0 / 60.0
    end

    self.time = self.time + dt

    local phase = (self.time * TRI_FREQ) % 2.0
    local tri = phase < 1.0 and phase or (2.0 - phase) -- 0..1..0
    local targetY = self.baseY + (tri * 2.0 - 1.0) * TRI_AMP
    local cur = Transform.getPosition(entity)
    Transform.translate(entity, -DRIFT_SPEED * dt, targetY - cur.y)

    local trail = _PataTrails["trail_" .. tostring(entity)]
    if trail == nil then
        trail = {}
        _PataTrails["trail_" .. tostring(entity)] = trail
    end
    local now = Transform.getPosition(entity)
    local last = trail[1]
    if last == nil or last.x ~= now.x or last.y ~= now.y then
        table.insert(trail, 1, { x = now.x, y = now.y })
    end

    local walked = 0.0
    local keep = 1
    while keep < #trail do
        local a, b = trail[keep], trail[keep + 1]
        local dx, dy = b.x - a.x, b.y - a.y
        walked = walked + math.sqrt(dx * dx + dy * dy)
        keep = keep + 1
        if walked >= TRAIL_KEEP_PX then
            break
        end
    end
    while #trail > keep do
        table.remove(trail)
    end
end

function Script:onDestroy(entity)
    log.info("[" .. tostring(entity) .. "] Script destroyed")
end

return Script
