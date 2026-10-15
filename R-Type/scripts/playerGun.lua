--- playerGun.lua Attached to: Gun (child of Player entity)
-- Features: Muzzle flash + shooting projectiles

local Script = {}

local MUZZLE_01 = "muzzleflash_000.png"
local MUZZLE_02 = "muzzleflash_001.png"
local FRAME_DURATION = 0.1

local PROJECTILE_SPRITE = "laserbeam.png"

local PROJECTILE_SPEED = 500.0 -- pixels per second
local PROJECTILE_LIFETIME = 3.0 -- seconds before auto-remove

-- Shooting state
function Script:onCreate(entity)
	self.entity = entity
	self.wasShootDown = false
	self.firing = false
	self.phase = 0
	self.elapsed = 0.0

	self.shootDown = false
	self.cooldown = 0.0
	self.projectiles = {}

	Component.setSpritePath(entity, MUZZLE_01)
	Component.setOpacity(entity, 0.0)

	log.info("[" .. tostring(entity) .. "] Script created")
end

function Script:onUpdate(entity, dt)
	if not dt or dt <= 0 then
		dt = 1.0 / 60.0
	end

	local shootDown = Controller.isActionActive("SHOOT")

	if shootDown and not self.wasShootDown and not self.firing then
		self.firing = true
		self.phase = 1
		self.elapsed = 0.0

		Component.setSpritePath(entity, MUZZLE_01)
		Component.setOpacity(entity, 1.0)
	end

	self.wasShootDown = shootDown

	if self.firing then
		self.elapsed = self.elapsed + dt

		if self.phase == 1 and self.elapsed >= FRAME_DURATION then
			self.elapsed = self.elapsed - FRAME_DURATION
			self.phase = 2
			Component.setSpritePath(entity, MUZZLE_02)
		end

		if self.phase == 2 and self.elapsed >= FRAME_DURATION then
			self.firing = false
			self.phase = 0
			self.elapsed = 0.0
			Component.setOpacity(entity, 0.0)
		end
	end

	self.cooldown = math.max(0, self.cooldown - dt)

	if shootDown and not self.shootDown and self.cooldown <= 0 then
		self:fireProjectile()
		self.cooldown = 0.3
	end

	self.shootDown = shootDown

	for i = #self.projectiles, 1, -1 do
		local proj = self.projectiles[i]
		local projEntity = proj.entity

		Transform.translate(projEntity, PROJECTILE_SPEED * dt, 0)

		proj.timer = proj.timer - dt
		if proj.timer <= 0 then
			Entity.destroy(projEntity)
			table.remove(self.projectiles, i)
		end
	end
end

function Script:fireProjectile()
	local projEntity = Entity.create()

	local gunPos = Transform.getPosition(self.entity)
	local gunPosX = gunPos.x
	local gunPosY = gunPos.y
	Transform.setPosition(projEntity, gunPosX, gunPosY + 6)
	Component.addSprite(projEntity, PROJECTILE_SPRITE, 0, 0, 4.0, 4.0)
	Component.addBoxCollider(projEntity, 32, 32)

	Component.setOpacity(projEntity, 1.0)

	table.insert(self.projectiles, {
		entity = projEntity,
		timer = PROJECTILE_LIFETIME,
	})

	log.info("[" .. tostring(entity) .. "] Fired projectile")
end

function Script:onDestroy(entity)
	for _, proj in ipairs(self.projectiles) do
		Entity.destroy(proj.entity)
	end
	self.projectiles = {}
	log.info("[" .. tostring(entity) .. "] Script destroyed")
end

return Script
