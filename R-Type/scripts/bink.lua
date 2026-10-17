-- bink.lua
-- Attached to: Bink
--
local Script = {}

local WALK_FRAMES = {
	"assets/enemies/bink/bink000.png",
	"assets/enemies/bink/bink001.png",
	"assets/enemies/bink/bink002.png",
}
local AIR_FRAME = 3
local FRAME_DURATION = 0.12

local WALK_SPEED = 90.0 -- pixels per second
local GRAVITY = 1200.0 -- pixels per second squared
local JUMP_SPEED = 430.0 -- upward impulse, hops about 77px
local JUMP_DELAY_MIN = 1.5 -- seconds between hops
local JUMP_DELAY_MAX = 3.5
local TURN_LOCKOUT = 0.35 -- ignore the sensor this long after a turn

local BODY_LAYER = "Enemy"
local BODY_MASK = Layers.bit("Player") + Layers.bit("PlayerBullet") + Layers.bit("Wall")
local SENSOR_MASK = Layers.bit("Wall")

-- Death plays the same explosion as every other enemy (enemy1.lua). Only a
-- real shot counts: floor, ceiling and stage blocks are all "Wall", and the
-- walker touches those constantly.
local EXPLOSION = {
	"assets/effects/enemy-explosion-1000.png",
	"assets/effects/enemy-explosion-1001.png",
	"assets/effects/enemy-explosion-1002.png",
	"assets/effects/enemy-explosion-1003.png",
	"assets/effects/enemy-explosion-1004.png",
	"assets/effects/enemy-explosion-1005.png",
}
local EXPLOSION_FRAME_DURATION = 0.1
local KILLER_LAYER = "PlayerBullet"

local function NextJumpDelay()
	return JUMP_DELAY_MIN + math.random() * (JUMP_DELAY_MAX - JUMP_DELAY_MIN)
end

function Script:onCreate(entity)
	self.entity = entity
	self.dir = -1
	self.vy = 0.0
	self.grounded = false
	self.frame = 1
	self.elapsed = 0.0
	self.jumpTimer = NextJumpDelay()
	self.turnLock = 0.0
	self.exploding = false

	-- The scene does not author a connection for this entity (the Pata-Patas
	-- have theirs written into test.scene), so the script wires its own death
	-- callback here. Component.connect is idempotent, so a script reload
	-- re-running onCreate does not duplicate the connection.
	Component.connect(entity, "onCollisionEnter", entity, "onCollisionEnter")

	Component.setSpritePath(entity, WALK_FRAMES[self.frame])
	Component.setColliderLayer(entity, BODY_LAYER)
	Component.setColliderMask(entity, BODY_MASK)

	self.sensor = nil
	for _, child in ipairs(Component.getChildren(entity)) do
		if Component.getColliderTrigger(child) then
			self.sensor = child
			Component.setColliderLayer(child, BODY_LAYER)
			Component.setColliderMask(child, SENSOR_MASK)
			break
		end
	end

	-- The sensor is authored in front of the body, so it decides which way we
	-- start walking (and which way the art has to face).
	if self.sensor ~= nil then
		local dx = Transform.getPosition(self.sensor).x - Transform.getPosition(entity).x
		self.dir = (dx >= 0) and 1 or -1
		Component.setSpriteFlip(entity, self.dir > 0)
	end
end

function Script:onUpdate(entity, dt)
	if not dt or dt <= 0 then
		dt = 1.0 / 60.0
	end

	-- Already dead: freeze where the shot landed (no gravity, no walking, no
	-- turning) and play the explosion out, exactly like enemy1.lua does.
	if self.exploding then
		self:AnimateExplosion(entity, dt)
		return
	end

	self.turnLock = math.max(0.0, self.turnLock - dt)

	-- Fall, and stop falling as soon as something holds the body up. While
	-- grounded gravity keeps pressing the body against the floor, which is what
	-- makes the contact stay alive frame after frame.
	self.vy = self.vy + GRAVITY * dt
	Transform.translate(entity, 0.0, self.vy * dt)
	if self.vy > 0.0 and Component.isColliding(entity) then
		self.vy = 0.0
		self.grounded = true
	end

	-- Hop every now and then, but only from the ground.
	self.jumpTimer = self.jumpTimer - dt
	if self.grounded and self.jumpTimer <= 0.0 then
		self.vy = -JUMP_SPEED
		self.grounded = false
		self.jumpTimer = NextJumpDelay()
	end

	Transform.translate(entity, WALK_SPEED * self.dir * dt, 0.0)

	-- Something is coming up in front: walk the other way.
	if self.turnLock <= 0.0 and self.sensor ~= nil and Component.isColliding(self.sensor) then
		self:TurnAround()
	end

	self:Animate(entity, dt)
end

function Script:TurnAround()
	self.dir = -self.dir
	self.turnLock = TURN_LOCKOUT
	-- Mirrors the sprite and the whole subtree, so the sensor ends up in front
	-- of the body on the new side.
	Component.flipX(self.entity)
end

function Script:onCollisionEnter(this, other)
	if self.exploding then
		return
	end
	-- Floor, ceiling and stage blocks all live on "Wall"; the walker is on top
	-- of the floor from the very first second, so contact alone must not kill
	-- it. The front sensor never sees bullets either (Wall-only mask).
	if Component.getColliderLayerName(other) ~= KILLER_LAYER then
		return
	end

	self.exploding = true
	self.frame = 1
	self.elapsed = 0.0
	Component.setSpritePath(this, EXPLOSION[self.frame])
end

function Script:AnimateExplosion(entity, dt)
	self.elapsed = self.elapsed + dt

	while self.elapsed >= EXPLOSION_FRAME_DURATION do
		self.elapsed = self.elapsed - EXPLOSION_FRAME_DURATION
		self.frame = self.frame + 1

		if self.frame > #EXPLOSION then
			Entity.queueDestroy(entity)
			return
		end

		Component.setSpritePath(entity, EXPLOSION[self.frame])
	end
end

function Script:Animate(entity, dt)
	if not self.grounded then
		-- Held pose while airborne. The walk timer restarts on landing instead
		-- of burning through a dozen frames at once.
		self.elapsed = 0.0
		if self.frame ~= AIR_FRAME then
			self.frame = AIR_FRAME
			Component.setSpritePath(entity, WALK_FRAMES[self.frame])
		end
		return
	end

	self.elapsed = self.elapsed + dt
	while self.elapsed >= FRAME_DURATION do
		self.elapsed = self.elapsed - FRAME_DURATION
		self.frame = (self.frame % #WALK_FRAMES) + 1
		Component.setSpritePath(entity, WALK_FRAMES[self.frame])
	end
end

return Script
