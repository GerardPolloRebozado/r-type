-- TODO: _PataTrails is a bad idea. The engine should give us a proper way
-- to do this instead: ordered children (getChildren order is random, hence
-- the sort below), custom signals a script can emit (so links subscribe to
-- group movement instead of re-reading a trail buffer every frame), and a
-- real shared-data object instead of a _G global. See Unity (UnityEvent,
-- ScriptableObjects) and Godot (signals, shared Resources, autoloads)
local Enemy = {}

local SPRITES = {
	"assets/enemies/red-spinner/enemy1000.png",
	"assets/enemies/red-spinner/enemy1001.png",
	"assets/enemies/red-spinner/enemy1002.png",
	"assets/enemies/red-spinner/enemy1003.png",
	"assets/enemies/red-spinner/enemy1004.png",
	"assets/enemies/red-spinner/enemy1005.png",
	"assets/enemies/red-spinner/enemy1006.png",
	"assets/enemies/red-spinner/enemy1007.png",
}

local EXPLOSION = {
	"assets/effects/enemy-explosion-1000.png",
	"assets/effects/enemy-explosion-1001.png",
	"assets/effects/enemy-explosion-1002.png",
	"assets/effects/enemy-explosion-1003.png",
	"assets/effects/enemy-explosion-1004.png",
	"assets/effects/enemy-explosion-1005.png",
}

local FRAME_DURATION = 0.1

local SPACING = 100.0

-- Link order along the trail: entity ids say nothing about where a link sits in
-- the formation, so sort by x - the group's drift is leftward, so left = in
-- front. Must match group-patapata.lua.
local function SortByX(ids)
	table.sort(ids, function(a, b)
		return Transform.getPosition(a).x < Transform.getPosition(b).x
	end)
	return ids
end

local function TrailPointAt(trail, dist)
	local walked = 0.0
	for i = 1, #trail - 1 do
		local a, b = trail[i], trail[i + 1]
		local dx, dy = b.x - a.x, b.y - a.y
		local seg = math.sqrt(dx * dx + dy * dy)
		if walked + seg >= dist then
			local t = (seg > 0) and ((dist - walked) / seg) or 0
			return { x = a.x + dx * t, y = a.y + dy * t }
		end
		walked = walked + seg
	end
	return nil
end

function Enemy:onCreate(entity)
	self.entity = entity
	self.frame = 1
	self.elapsed = 0.0
	self.exploding = false

	Component.setSpritePath(entity, SPRITES[self.frame])
	Component.addBoxCollider(entity, 32, 32)

	self.linkIndex = nil
	self.trailKey = nil
	local parent = Component.getParent(entity)
	if parent ~= nil then
		self.trailKey = "trail_" .. tostring(parent)
		local sibs = Component.getChildren(parent)
		SortByX(sibs)
		for i, id in ipairs(sibs) do
			if id == entity then
				self.linkIndex = i
				break
			end
		end
	end
end

function Enemy:onUpdate(entity, dt)
	if not dt or dt <= 0 then
		dt = 1.0 / 60.0
	end

	if self.linkIndex ~= nil and self.trailKey ~= nil then
		local bus = _PataTrails
		local trail = (bus ~= nil) and bus[self.trailKey] or nil
		if trail ~= nil then
			local point = TrailPointAt(trail, self.linkIndex * SPACING)
			if point ~= nil then
				Transform.setPosition(entity, point.x, point.y)
			end
		end
	end

	if self.exploding then
		self.elapsed = self.elapsed + dt

		while self.elapsed >= FRAME_DURATION do
			self.elapsed = self.elapsed - FRAME_DURATION
			self.frame = self.frame + 1

			if self.frame > #EXPLOSION then
				Entity.queueDestroy(entity)
				return
			end

			Component.setSpritePath(entity, EXPLOSION[self.frame])
		end

		return
	end

	self.elapsed = self.elapsed + dt

	while self.elapsed >= FRAME_DURATION do
		self.elapsed = self.elapsed - FRAME_DURATION
		self.frame = (self.frame % #SPRITES) + 1
		Component.setSpritePath(entity, SPRITES[self.frame])
	end
end

function Enemy:onCollisionEnter(this, other)
	if self.exploding then
		return
	end

	self.exploding = true
	self.frame = 1
	self.elapsed = 0.0

	Component.setSpritePath(this, EXPLOSION[self.frame])
end

return Enemy
