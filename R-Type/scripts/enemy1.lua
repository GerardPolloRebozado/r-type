local Enemy = {}

local SPRITES = {
	"assets/enemies/red-spinner/enemy1000.png",
	"assets/enemies/red-spinner/enemy1001.png",
	"assets/enemies/red-spinner/enemy1002.png",
	"assets/enemies/red-spinner/enemy1003.png",
	"assets/enemies/red-spinner/enemy1004.png",
	"assets/enemies/red-spinner/enemy1005.png",
	"assets/enemies/red-spinner/enemy1005.png",
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

-- Only the server (and single-player) simulates replicated enemies. On a
-- networked client, ReplicationSystem applies the server state. Entities
-- without a NetworkComponent (no network id) still simulate locally.
local function shouldSimulate(entity)
	if Net == nil or not Net.isNetworked() then
		return true
	end
	if Net.isServer() then
		return true
	end
	return Net.networkId(entity) == nil
end

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

	-- Without this the collision callback is never invoked, so the enemy can
	-- never be destroyed. bink.lua does the same in its onCreate.
	Component.connect(entity, "onCollisionEnter", entity, "onCollisionEnter")

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
	if not shouldSimulate(entity) then
		return
	end

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
	if not shouldSimulate(this) then
		return
	end

	if self.exploding then
		return
	end

	-- Only die from player bullets, not from walls/floor
	if Component.getColliderLayerName(other) ~= "PlayerBullet" then
		return
	end

	self.exploding = true
	self.frame = 1
	self.elapsed = 0.0

	Component.setSpritePath(this, EXPLOSION[self.frame])
end

return Enemy
