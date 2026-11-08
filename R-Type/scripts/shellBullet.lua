local Bullet = {}

local DIR = "assets/enemies/shell/bullets/"
local FRAMES = {}
for i = 0, 7 do
	FRAMES[#FRAMES + 1] = string.format("%sbullet%03d.png", DIR, i)
end
local FRAME_DURATION = 0.06
local SPEED = 220.0
local LIFETIME = 8.0
local SCREEN_MARGIN = 128.0

local function shouldSimulate(entity)
	if Net == nil or not Net.isNetworked() then
		return true
	end
	if Net.isServer() then
		return true
	end
	return Net.networkId(entity) == nil
end

local function center(entity)
	local pos = Transform.getPosition(entity)
	local size = Component.getColliderSize(entity)
	if size == nil then
		return pos.x, pos.y
	end
	return pos.x + size.width * 0.5, pos.y + size.height * 0.5
end

local function closestPlayer(x, y)
	local best, bestDist = nil, math.huge
	for _, entity in ipairs(Component.entitiesWith("Collider")) do
		if Component.getColliderLayerName(entity) == "Player" and Component.getColliderEnabled(entity) then
			local px, py = center(entity)
			local dist = (px - x) * (px - x) + (py - y) * (py - y)
			if dist < bestDist then
				best, bestDist = entity, dist
			end
		end
	end
	return best
end

function Bullet:onCreate(entity)
	self.entity = entity
	self.timer = LIFETIME
	self.frame = 1
	self.elapsed = 0.0
	self.vx, self.vy = -SPEED, 0.0

	Component.connect(entity, "onCollisionEnter", entity, "onCollisionEnter")

	local x, y = center(entity)
	local target = closestPlayer(x, y)
	if target ~= nil then
		local tx, ty = center(target)
		local dx, dy = tx - x, ty - y
		local len = math.sqrt(dx * dx + dy * dy)
		if len > 0.001 then
			self.vx, self.vy = dx / len * SPEED, dy / len * SPEED
		end
	end
end

function Bullet:onUpdate(entity, dt)
	if not shouldSimulate(entity) then
		return
	end
	if not dt or dt <= 0 then
		dt = 1.0 / 60.0
	end

	Transform.translate(entity, self.vx * dt, self.vy * dt)

	self.elapsed = self.elapsed + dt
	while self.elapsed >= FRAME_DURATION do
		self.elapsed = self.elapsed - FRAME_DURATION
		self.frame = (self.frame % #FRAMES) + 1
		Component.setSpritePath(entity, FRAMES[self.frame])
	end

	self.timer = self.timer - dt
	local outside = false
	if Camera ~= nil and Camera.viewport ~= nil then
		local pos = Transform.getPosition(entity)
		local vp = Camera.viewport()
		outside = pos.x < vp.x - SCREEN_MARGIN or
			pos.x > vp.x + vp.width + SCREEN_MARGIN or
			pos.y < vp.y - SCREEN_MARGIN or
			pos.y > vp.y + vp.height + SCREEN_MARGIN
	end
	if self.timer <= 0.0 or outside then
		Entity.queueDestroy(entity)
	end
end

function Bullet:onCollisionEnter(this, other)
	if not shouldSimulate(this) then
		return
	end
	if Component.getColliderLayerName(other) == "Player" then
		Entity.queueDestroy(this)
	end
end

return Bullet
