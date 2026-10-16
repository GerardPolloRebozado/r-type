-- group-patapata.lua
-- Attached to: Grouped Pata-Pata
--

local DRIFT_SPEED = 130.0 -- pixels per second, leftward
local TRI_AMP = 80.0 -- vertical zigzag half-height in pixels
local TRI_FREQ = 0.9 -- zigzag cycles per second

local TRAIL_KEEP_PX = 1500.0

local Script = {}

local function SeedTrail(entity)
	_PataTrails = _PataTrails or {}
	local key = "trail_" .. tostring(entity)
	local trail = {}
	_PataTrails[key] = trail

	local pos = Transform.getPosition(entity)
	table.insert(trail, { x = pos.x, y = pos.y })

	local sibs = Component.getChildren(entity) or {}
	table.sort(sibs)
	for _, id in ipairs(sibs) do
		local p = Transform.getPosition(id)
		table.insert(trail, { x = p.x, y = p.y })
	end

	local dirX, dirY = 1.0, 0.0
	if #trail >= 2 then
		local a, b = trail[#trail - 1], trail[#trail]
		local dx, dy = b.x - a.x, b.y - a.y
		local len = math.sqrt(dx * dx + dy * dy)
		if len > 0.001 then
			dirX, dirY = dx / len, dy / len
		end
	end
	local covered = 0.0
	for i = 1, #trail - 1 do
		local a, b = trail[i], trail[i + 1]
		local dx, dy = b.x - a.x, b.y - a.y
		covered = covered + math.sqrt(dx * dx + dy * dy)
	end
	local tip = trail[#trail]
	local need = (#sibs + 2) * 90.0
	while covered < need do
		tip = { x = tip.x + dirX * 90.0, y = tip.y + dirY * 90.0 }
		table.insert(trail, tip)
		covered = covered + 90.0
	end
end

function Script:onCreate(entity)
	self.entity = entity
	self.time = 0.5 / TRI_FREQ
	local pos = Transform.getPosition(entity)
	self.baseY = pos.y
	SeedTrail(entity)
	log.info("[" .. tostring(entity) .. "] Script created")
end

function Script:onUpdate(entity, dt)
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
