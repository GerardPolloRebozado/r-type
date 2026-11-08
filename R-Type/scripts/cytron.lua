-- cytron.lua
-- Attached to: Cytron
--
-- Crosses the screen right to left while weaving up and down in a wave.
-- Its face always points away from where it is moving, and it drops a trail
-- of green dots (see cytronDot.lua) that kill the player on contact.
local Cytron = {}

-- Facing frames, one every 22.5 degrees counter-clockwise starting at "face
-- right" (index 1). Row 2 of the sheet covers the upper half (013 -> 007),
-- row 1 the lower half (006 -> 000). If a direction looks wrong in game,
-- reorder this table.
local FRAME_DIR = "assets/enemies/cytron/"
local FACING_FRAMES = {
	FRAME_DIR .. "cytron015.png", --   0 deg, right
	FRAME_DIR .. "cytron013.png", --  22.5
	FRAME_DIR .. "cytron012.png", --  45
	FRAME_DIR .. "cytron011.png", --  67.5
	FRAME_DIR .. "cytron010.png", --  90 deg, up
	FRAME_DIR .. "cytron009.png", -- 112.5
	FRAME_DIR .. "cytron008.png", -- 135
	FRAME_DIR .. "cytron007.png", -- 157.5
	FRAME_DIR .. "cytron014.png", -- 180 deg, left
	FRAME_DIR .. "cytron006.png", -- 202.5
	FRAME_DIR .. "cytron005.png", -- 225
	FRAME_DIR .. "cytron004.png", -- 247.5
	FRAME_DIR .. "cytron003.png", -- 270 deg, down
	FRAME_DIR .. "cytron002.png", -- 292.5
	FRAME_DIR .. "cytron001.png", -- 315
	FRAME_DIR .. "cytron000.png", -- 337.5
}
local SECTOR = (2.0 * math.pi) / #FACING_FRAMES

-- true: the face points opposite to the motion. Set to false to flip it.
local FACE_AWAY_FROM_MOTION = true

local SPEED_X = 120.0 -- pixels per second, towards the left
local WAVE_AMPLITUDE = 120.0 -- pixels above and below the start height
local WAVE_FREQUENCY = 2.0 -- radians per second

local DOT_SPRITE = FRAME_DIR .. "cytron016.png"
local DOT_SCRIPT = "scripts/cytronDot.lua"
local DOT_INTERVAL = 0.25 -- seconds between two dots
local DOT_SCALE = 4.0
local DOT_SIZE = 32.0 -- collider side, matches the 8px sprite at scale 4

local SCREEN_MARGIN = 128.0

local BODY_LAYER = "Enemy"
local BODY_MASK = Layers.bit("Player") + Layers.bit("PlayerBullet") + Layers.bit("Wall")
local DOT_MASK = Layers.bit("Player") + Layers.bit("PlayerBullet")

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

local function isNetworked()
	return Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked()
end

-- Index into FACING_FRAMES for a direction given in screen space (y down).
local function FacingIndex(dx, dy)
	local angle = math.atan(-dy, dx) -- flip y so angles go counter-clockwise
	local sector = math.floor(angle / SECTOR + 0.5) % #FACING_FRAMES
	return sector + 1
end

function Cytron:onCreate(entity)
	self.entity = entity
	self.exploding = false
	self.frame = 1
	self.elapsed = 0.0
	self.time = 0.0
	self.dotTimer = DOT_INTERVAL
	self.facing = nil

	local start = Transform.getPosition(entity)
	self.x = start.x
	self.baseY = start.y

	Component.connect(entity, "onCollisionEnter", entity, "onCollisionEnter")
	Component.setColliderLayer(entity, BODY_LAYER)
	Component.setColliderMask(entity, BODY_MASK)
	Component.addAudioSource(entity, "assets/audio/explosion.mp3", false, false, 1)

	if isNetworked() then
		Net.replicate(entity, "Server", true, true)
	end

	self:Face(entity, -SPEED_X, WAVE_AMPLITUDE * WAVE_FREQUENCY)
end

function Cytron:onUpdate(entity, dt)
	if not shouldSimulate(entity) then
		return
	end

	if not dt or dt <= 0 then
		dt = 1.0 / 60.0
	end

	if self.exploding then
		self:AnimateExplosion(entity, dt)
		return
	end

	self.time = self.time + dt
	local vx = -SPEED_X
	local vy = WAVE_AMPLITUDE * WAVE_FREQUENCY * math.cos(WAVE_FREQUENCY * self.time)
	self.x = self.x + vx * dt
	local y = self.baseY + WAVE_AMPLITUDE * math.sin(WAVE_FREQUENCY * self.time)
	Transform.setPosition(entity, self.x, y)

	self:Face(entity, vx, vy)

	local viewport = nil
	if Camera ~= nil and Camera.viewport ~= nil then
		viewport = Camera.viewport()
	end

	-- Only leave a trail once on screen: dots dropped off-screen to the right
	-- are never seen but still cost a script, a sprite and a collider each.
	local onScreen = viewport == nil or self.x <= viewport.x + viewport.width
	self.dotTimer = self.dotTimer - dt
	while self.dotTimer <= 0.0 do
		if onScreen then
			self:DropDot(self.x, y)
		end
		self.dotTimer = self.dotTimer + DOT_INTERVAL
	end

	if viewport ~= nil then
		if self.x < viewport.x - SCREEN_MARGIN then
			Entity.queueDestroy(entity)
		end
	end
end

function Cytron:Face(entity, vx, vy)
	local dx, dy = vx, vy
	if FACE_AWAY_FROM_MOTION then
		dx, dy = -vx, -vy
	end

	local index = FacingIndex(dx, dy)
	if index ~= self.facing then
		self.facing = index
		Component.setSpritePath(entity, FACING_FRAMES[index])
	end
end

function Cytron:DropDot(x, y)
	local dot = Entity.create()
	Entity.setName(dot, "CytronDot")
	Transform.setPosition(dot, x, y, -1) -- drawn behind the Cytron
	Component.addSprite(dot, DOT_SPRITE, 0, 0, DOT_SCALE, DOT_SCALE)
	Component.addBoxCollider(dot, DOT_SIZE, DOT_SIZE)
	Component.setColliderLayer(dot, BODY_LAYER)
	Component.setColliderMask(dot, DOT_MASK)
	Component.addScript(dot, DOT_SCRIPT)

	if isNetworked() then
		Net.replicate(dot, "Server", true, true)
	end
end

function Cytron:onCollisionEnter(this, other)
	if not shouldSimulate(this) then
		return
	end

	if self.exploding then
		return
	end
	if Component.getColliderLayerName(other) ~= KILLER_LAYER then
		return
	end

	-- The dots already dropped stay on screen: they have their own script.
	self.exploding = true
	self.frame = 1
	self.elapsed = 0.0
	Component.playAudio(this)
	Component.setSpritePath(this, EXPLOSION[self.frame])
end

function Cytron:AnimateExplosion(entity, dt)
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

return Cytron
