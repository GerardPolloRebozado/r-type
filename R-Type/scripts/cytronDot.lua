-- cytronDot.lua
-- Attached at runtime by cytron.lua to every green dot of the Cytron's trail.
--
-- A dot stays where it was dropped. It kills the player on contact (the
-- player script does that, since dots are on the "Enemy" layer) and breaks
-- when a player shot hits it. It disappears after LIFETIME seconds, or once
-- it scrolls off the left, top or bottom of the screen. It has its own script
-- so it survives the Cytron's death.
local Dot = {}

local SCREEN_MARGIN = 64.0
-- Seconds before a dot fades out on its own. Keeps the trail short so dots
-- don't pile up (hundreds of them slow the game down a lot).
local LIFETIME = 3.0

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

function Dot:onCreate(entity)
	self.entity = entity
	self.destroyed = false
	self.age = 0.0
	Component.connect(entity, "onCollisionEnter", entity, "onCollisionEnter")
end

function Dot:onUpdate(entity, dt)
	if self.destroyed or not shouldSimulate(entity) then
		return
	end

	if not dt or dt <= 0 then
		dt = 1.0 / 60.0
	end
	self.age = self.age + dt
	if self.age >= LIFETIME then
		self:Break(entity)
		return
	end

	if Camera == nil or Camera.viewport == nil then
		return
	end

	local position = Transform.getPosition(entity)
	local viewport = Camera.viewport()
	if position.x < viewport.x - SCREEN_MARGIN or
		position.y < viewport.y - SCREEN_MARGIN or
		position.y > viewport.y + viewport.height + SCREEN_MARGIN then
		self:Break(entity)
	end
end

function Dot:onCollisionEnter(this, other)
	if self.destroyed or not shouldSimulate(this) then
		return
	end

	local layer = Component.getColliderLayerName(other)
	if layer == "PlayerBullet" or layer == "Player" then
		self:Break(this)
	end
end

function Dot:Break(entity)
	self.destroyed = true
	Entity.queueDestroy(entity)
end

return Dot
