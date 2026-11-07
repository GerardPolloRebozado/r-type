-- bulletProjectile.lua
local BulletProjectile = {}

local SPEED = 700.0
local LIFETIME = 3.0
local SCREEN_MARGIN = 128.0
local SPRITE = "assets/enemies/red-spinner/laserbeam.png"

function BulletProjectile:onCreate(entity)
	self.entity = entity
	self.timer = LIFETIME
end

function BulletProjectile:onUpdate(entity, dt)
	if not dt or dt <= 0 then
		dt = 1.0 / 60.0
	end

	Transform.translate(entity, -SPEED * dt, 0)
	self.timer = self.timer - dt

	local pos = Transform.getPosition(entity)
	local outside = pos.x < -SCREEN_MARGIN or pos.x > 100000
	if self.timer <= 0 or outside then
		Entity.queueDestroy(entity)
	end
end

function BulletProjectile:onCollisionEnter(this, other)
	local layer = Component.getColliderLayerName(other)
	if layer == "Player" or layer == "Wall" or layer == "PlayerBullet" then
		Entity.queueDestroy(this)
	end
end

return BulletProjectile