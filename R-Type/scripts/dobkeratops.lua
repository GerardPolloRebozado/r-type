-- dobkeratops.lua
-- Attached to: Dobkeratops (boss)
--
-- A screen-tall boss locked to the right side of the camera. It is built
-- from several entities that this script creates and moves every frame:
--   - the body (this entity): sprite only, the mouth animates;
--   - the core: the only weak spot, fires bullets (dobkeratopsBullet.lua);
--   - body hitboxes: invisible boxes following the crescent shape, they
--     block player shots and hurt the player on touch;
--   - the tail: a chain of balls ending in a tip, sweeping the screen. It
--     hurts the player and blocks shots but cannot be destroyed.
-- Damage to the player is handled by player.lua (Enemy / EnemyBullet layers).
local Boss = {}

local DIR = "assets/enemies/dobkeratops/"

-- All offsets below are in sheet pixels (body frame is 155x204); they get
-- multiplied by SCALE. Sprites and colliders are anchored at their top-left.
local SCALE = 5.0
local BODY_W, BODY_H = 155, 204

local MAX_HEALTH = 20

local BODY_FRAMES = {
	DIR .. "dobkeratops000.png",
	DIR .. "dobkeratops001.png",
	DIR .. "dobkeratops002.png",
	DIR .. "dobkeratops003.png",
}
local BODY_FRAME_DURATION = 0.18

-- Core frames, in the order they appear in the sheet's body rows.
local CORE_EMERGE = {
	DIR .. "dobkeratops-core005.png", -- closed skin
	DIR .. "dobkeratops-core004.png", -- hole opens
	DIR .. "dobkeratops-core003.png",
	DIR .. "dobkeratops-core002.png",
	DIR .. "dobkeratops-core001.png",
	DIR .. "dobkeratops-core000.png", -- core fully out
}
local CORE_IDLE = {
	DIR .. "dobkeratops-core000.png",
	DIR .. "dobkeratops-core007.png",
	DIR .. "dobkeratops-core006.png",
	DIR .. "dobkeratops-core007.png",
}
local CORE_FRAME_DURATION = 0.15
local CORE_X, CORE_Y, CORE_SIZE = 72, 94, 32
local CORE_HIT_BLINK = 0.25 -- seconds the core blinks after a hit

-- Invisible boxes roughly covering the crescent, without covering the core.
-- { x, y, width, height } in sheet pixels.
local BODY_BOXES = {
	{ 0, 0, 110, 60 }, -- head
	{ 78, 60, 77, 34 }, -- neck
	{ 104, 94, 51, 32 }, -- spine, right of the core
	{ 88, 126, 67, 48 }, -- lower body
	{ 54, 174, 71, 30 }, -- lower end, where the tail starts
}

-- Tail: 7 large, 6 medium, 5 small balls, then the tip.
local TAIL_SPRITES = {
	DIR .. "dobkeratops-tail000.png",
	DIR .. "dobkeratops-tail001.png",
	DIR .. "dobkeratops-tail002.png",
}
local TAIL_LAYOUT = { 7, 6, 5 } -- how many balls of each size
local TAIL_BALL_SIZE = { 16, 14, 12 } -- visible size in sheet pixels
local TIP_FRAMES = {}
for i = 0, 8 do
	TIP_FRAMES[#TIP_FRAMES + 1] = string.format("%sdobkeratops-tip%03d.png", DIR, i)
end
local TIP_SIZE = 16
local TIP_FRAME_DURATION = 0.08
local TAIL_SPRITE_SIZE = 16 -- every tail png is a 16x16 canvas
local TAIL_ANCHOR_X, TAIL_ANCHOR_Y = 62, 188 -- tail root on the body
local TAIL_SPACING = 0.8 -- distance between balls, relative to their size
local TAIL_BASE_ANGLE = math.pi * 0.85 -- root points left and slightly down
local TAIL_SWING = 0.9 -- radians the whole tail swings around its root
local TAIL_SWING_SPEED = 0.7
local TAIL_CURL = 0.16 -- extra bend added at every segment
local TAIL_CURL_SPEED = 1.6
local TAIL_CURL_PHASE = 0.35

-- Bullets
local BULLET_SCRIPT = "scripts/dobkeratopsBullet.lua"
local BULLET_SPRITE = DIR .. "dobkeratops-bullet000.png"
local BULLET_SCALE = 3.0
local BULLET_SIZE = 22 * BULLET_SCALE * 0.7 -- a bit smaller than the sprite
local FIRE_INTERVAL = 0.6 -- aimed shot at the closest player
local RING_INTERVAL = 2.2 -- burst of bullets in every direction
local RING_COUNT = 16 -- bullets per burst
local RING_TURN = math.pi / 16 -- each burst is rotated so the gaps move

local ENTER_DURATION = 2.5 -- slide in from the right
local EMERGE_FRAME_DURATION = 0.25

local EXPLOSION = {
	"assets/effects/enemy-explosion-1000.png",
	"assets/effects/enemy-explosion-1001.png",
	"assets/effects/enemy-explosion-1002.png",
	"assets/effects/enemy-explosion-1003.png",
	"assets/effects/enemy-explosion-1004.png",
	"assets/effects/enemy-explosion-1005.png",
}
local EXPLOSION_FRAME_DURATION = 0.1
local EXPLOSION_SIZE = 32 -- explosion pngs are 32x32
local EXPLOSION_SCALE = 4.0
local DEATH_DURATION = 2.0
local DEATH_BLAST_INTERVAL = 0.12

local PART_LAYER = "Enemy"
local PART_MASK = Layers.bit("Player") + Layers.bit("PlayerBullet")
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

local function replicate(entity, syncSprite)
	if isNetworked() then
		Net.replicate(entity, "Server", true, syncSprite)
	end
end

local function viewport()
	if Camera ~= nil and Camera.viewport ~= nil then
		return Camera.viewport()
	end
	return { x = 0, y = 0, width = 1920, height = 1080 }
end

-- Creates a part on the Enemy layer. sprite may be nil (invisible hitbox).
local function makePart(name, sprite, scale, width, height, z)
	local part = Entity.create()
	Entity.setName(part, name)
	Transform.setPosition(part, 0, 0, z or 0)
	if sprite ~= nil then
		Component.addSprite(part, sprite, 0, 0, scale, scale)
	end
	Component.addBoxCollider(part, width, height)
	Component.setColliderLayer(part, PART_LAYER)
	Component.setColliderMask(part, PART_MASK)
	replicate(part, sprite ~= nil)
	return part
end

function Boss:onCreate(entity)
	self.entity = entity
	self.state = "enter"
	self.health = MAX_HEALTH
	self.time = 0.0
	self.stateTime = 0.0
	self.bodyFrame = 1
	self.bodyElapsed = 0.0
	self.coreFrame = 1
	self.coreElapsed = 0.0
	self.tipFrame = 1
	self.tipElapsed = 0.0
	self.fireTimer = FIRE_INTERVAL
	self.ringTimer = RING_INTERVAL
	self.ringAngle = 0.0
	self.blinkTimer = 0.0
	self.blasts = {}
	self.blastTimer = 0.0

	Component.addAudioSource(entity, "assets/audio/explosion.mp3", false, false, 1)
	Component.setSpriteScale(entity, SCALE, SCALE)
	Component.setSpritePath(entity, BODY_FRAMES[1])
	replicate(entity, true)

	if not shouldSimulate(entity) then
		return
	end

	-- Start just off the right edge of the screen, centered vertically.
	local vp = viewport()
	self.screenX = vp.width
	self.screenY = (vp.height - BODY_H * SCALE) * 0.5
	self.targetScreenX = vp.width - BODY_W * SCALE

	self.core = makePart("DobkeratopsCore", CORE_EMERGE[1], SCALE,
		CORE_SIZE * SCALE, CORE_SIZE * SCALE, 1)
	Component.connect(self.core, "onCollisionEnter", entity, "onCoreHit")

	self.boxes = {}
	for i, box in ipairs(BODY_BOXES) do
		self.boxes[i] = makePart("DobkeratopsBody", nil, 1.0,
			box[3] * SCALE, box[4] * SCALE)
	end

	-- Tail balls, root first. The tip is the last segment.
	self.tail = {}
	-- Tail pngs are drawn centered on a 16x16 canvas: shift the sprite so it
	-- lines up with a collider that only covers the visible ball.
	local canvas = TAIL_SPRITE_SIZE * SCALE
	for kind, count in ipairs(TAIL_LAYOUT) do
		local size = TAIL_BALL_SIZE[kind] * SCALE
		local shift = -(canvas - size) * 0.5
		for _ = 1, count do
			local ball = makePart("DobkeratopsTail", TAIL_SPRITES[kind], SCALE, size, size, -1)
			Component.setSpriteOffset(ball, shift, shift)
			table.insert(self.tail, { entity = ball, size = size })
		end
	end
	local tipSize = TIP_SIZE * SCALE
	self.tip = makePart("DobkeratopsTailTip", TIP_FRAMES[1], SCALE, tipSize, tipSize, -1)
	table.insert(self.tail, { entity = self.tip, size = tipSize })

	self:PlaceParts(entity)
end

function Boss:onUpdate(entity, dt)
	if not shouldSimulate(entity) then
		return
	end
	if not dt or dt <= 0 then
		dt = 1.0 / 60.0
	end

	self.time = self.time + dt
	self.stateTime = self.stateTime + dt

	if self.state == "enter" then
		self:UpdateEnter(dt)
	elseif self.state == "fight" then
		self:UpdateFight(dt)
	elseif self.state == "dying" then
		self:UpdateDying(entity, dt)
		if self.state == "dead" then
			return
		end
	end

	self:PlaceParts(entity)
end

function Boss:UpdateEnter(dt)
	-- Slide in, then open the core. The core can't be hurt yet.
	local t = math.min(1.0, self.stateTime / ENTER_DURATION)
	local ease = 1.0 - (1.0 - t) * (1.0 - t)
	local vp = viewport()
	self.screenX = vp.width + (self.targetScreenX - vp.width) * ease

	if t >= 1.0 then
		self.coreElapsed = self.coreElapsed + dt
		while self.coreElapsed >= EMERGE_FRAME_DURATION do
			self.coreElapsed = self.coreElapsed - EMERGE_FRAME_DURATION
			if self.coreFrame >= #CORE_EMERGE then
				self:SetState("fight")
				self.coreFrame = 1
				return
			end
			self.coreFrame = self.coreFrame + 1
			Component.setSpritePath(self.core, CORE_EMERGE[self.coreFrame])
		end
	end
end

function Boss:UpdateFight(dt)
	self.bodyElapsed = self.bodyElapsed + dt
	while self.bodyElapsed >= BODY_FRAME_DURATION do
		self.bodyElapsed = self.bodyElapsed - BODY_FRAME_DURATION
		self.bodyFrame = (self.bodyFrame % #BODY_FRAMES) + 1
		Component.setSpritePath(self.entity, BODY_FRAMES[self.bodyFrame])
	end

	self.coreElapsed = self.coreElapsed + dt
	while self.coreElapsed >= CORE_FRAME_DURATION do
		self.coreElapsed = self.coreElapsed - CORE_FRAME_DURATION
		self.coreFrame = (self.coreFrame % #CORE_IDLE) + 1
		Component.setSpritePath(self.core, CORE_IDLE[self.coreFrame])
	end

	if self.blinkTimer > 0.0 then
		self.blinkTimer = math.max(0.0, self.blinkTimer - dt)
		local visible = self.blinkTimer <= 0.0 or (math.floor(self.blinkTimer * 20) % 2) == 0
		Component.setOpacity(self.core, visible and 1.0 or 0.2)
	end

	self:AnimateTip(dt)

	self.fireTimer = self.fireTimer - dt
	if self.fireTimer <= 0.0 then
		self.fireTimer = self.fireTimer + FIRE_INTERVAL
		self:Fire(nil)
	end

	self.ringTimer = self.ringTimer - dt
	if self.ringTimer <= 0.0 then
		self.ringTimer = self.ringTimer + RING_INTERVAL
		for i = 0, RING_COUNT - 1 do
			self:Fire(self.ringAngle + i * (2.0 * math.pi / RING_COUNT))
		end
		self.ringAngle = self.ringAngle + RING_TURN
	end
end

function Boss:SetState(state)
	self.state = state
	self.stateTime = 0.0
end

-- Moves every part to follow the body, which follows the camera.
function Boss:PlaceParts(entity)
	local vp = viewport()
	local bx = vp.x + self.screenX
	local by = vp.y + self.screenY
	Transform.setPosition(entity, bx, by)

	if self.core ~= nil then
		Transform.setPosition(self.core, bx + CORE_X * SCALE, by + CORE_Y * SCALE, 1)
	end

	if self.boxes ~= nil then
		for i, box in ipairs(BODY_BOXES) do
			Transform.setPosition(self.boxes[i], bx + box[1] * SCALE, by + box[2] * SCALE)
		end
	end

	if self.tail ~= nil then
		self:PlaceTail(bx + TAIL_ANCHOR_X * SCALE, by + TAIL_ANCHOR_Y * SCALE)
	end
end

-- Each segment bends a bit more than the previous one, with a wave running
-- down the chain, so the tail whips around like a snake.
function Boss:PlaceTail(x, y)
	local angle = TAIL_BASE_ANGLE + TAIL_SWING * math.sin(self.time * TAIL_SWING_SPEED)
	local prevSize = 0.0
	for i, segment in ipairs(self.tail) do
		if i > 1 then
			angle = angle + TAIL_CURL * math.sin(self.time * TAIL_CURL_SPEED - i * TAIL_CURL_PHASE)
			local step = (prevSize + segment.size) * 0.5 * TAIL_SPACING
			x = x + math.cos(angle) * step
			y = y + math.sin(angle) * step
		end
		prevSize = segment.size
		-- (x, y) is the ball center; the collider is anchored at its top-left.
		Transform.setPosition(segment.entity, x - segment.size * 0.5, y - segment.size * 0.5, -1)
	end
end

function Boss:AnimateTip(dt)
	self.tipElapsed = self.tipElapsed + dt
	while self.tipElapsed >= TIP_FRAME_DURATION do
		self.tipElapsed = self.tipElapsed - TIP_FRAME_DURATION
		self.tipFrame = (self.tipFrame % #TIP_FRAMES) + 1
		Component.setSpritePath(self.tip, TIP_FRAMES[self.tipFrame])
	end
end

-- angle (radians, screen space) sets the bullet direction; nil aims it at
-- the closest player.
function Boss:Fire(angle)
	local corePos = Transform.getPosition(self.core)
	local bullet = Entity.create()
	Entity.setName(bullet, "DobkeratopsBullet")
	-- Spawn on the core's center; the sprite is shifted to stay centered on
	-- the (smaller) collider.
	local half = CORE_SIZE * SCALE * 0.5
	local shift = -(22 * BULLET_SCALE - BULLET_SIZE) * 0.5
	Transform.setPosition(bullet, corePos.x + half - BULLET_SIZE * 0.5, corePos.y + half - BULLET_SIZE * 0.5, 2)
	Component.addSprite(bullet, BULLET_SPRITE, shift, shift, BULLET_SCALE, BULLET_SCALE)
	Component.addBoxCollider(bullet, BULLET_SIZE, BULLET_SIZE)
	Component.setColliderLayer(bullet, "EnemyBullet")
	Component.setColliderMask(bullet, Layers.bit("Player"))
	-- Read (and cleared) by dobkeratopsBullet.lua when it starts.
	if angle ~= nil then
		DobkeratopsBulletAngles = DobkeratopsBulletAngles or {}
		DobkeratopsBulletAngles[bullet] = angle
	end
	-- Own script so bullets keep flying after the boss dies.
	Component.addScript(bullet, BULLET_SCRIPT, true)
	replicate(bullet, true)
end

function Boss:onCoreHit(core, other)
	if not shouldSimulate(self.entity) then
		return
	end
	if self.state ~= "fight" then
		return
	end
	if Component.getColliderLayerName(other) ~= KILLER_LAYER then
		return
	end

	self.health = self.health - 1
	self.blinkTimer = CORE_HIT_BLINK
	log.info(string.format("[dobkeratops.lua] core hit, %d/%d health left", self.health, MAX_HEALTH))

	if self.health <= 0 then
		self:StartDying()
	end
end

function Boss:StartDying()
	self:SetState("dying")
	self.blastTimer = 0.0
	Component.setOpacity(self.core, 1.0)
	-- Nothing can hurt the player anymore while it explodes.
	for _, part in ipairs(self:Parts()) do
		Component.setColliderEnabled(part, false)
	end
	Component.playAudio(self.entity)
end

function Boss:UpdateDying(entity, dt)
	-- Spawn explosions at random spots on the body and the tail.
	self.blastTimer = self.blastTimer - dt
	if self.stateTime < DEATH_DURATION and self.blastTimer <= 0.0 then
		self.blastTimer = DEATH_BLAST_INTERVAL
		self:SpawnBlast()
	end

	for i = #self.blasts, 1, -1 do
		local blast = self.blasts[i]
		blast.elapsed = blast.elapsed + dt
		local frame = math.floor(blast.elapsed / EXPLOSION_FRAME_DURATION) + 1
		if frame > #EXPLOSION then
			Entity.queueDestroy(blast.entity)
			table.remove(self.blasts, i)
		elseif frame ~= blast.frame then
			blast.frame = frame
			Component.setSpritePath(blast.entity, EXPLOSION[frame])
		end
	end

	-- Fade the boss out during the second half of the explosions.
	local fade = 1.0 - math.max(0.0, (self.stateTime - DEATH_DURATION * 0.5) / (DEATH_DURATION * 0.5))
	Component.setOpacity(entity, math.max(0.0, fade))
	for _, segment in ipairs(self.tail) do
		Component.setOpacity(segment.entity, math.max(0.0, fade))
	end
	Component.setOpacity(self.core, math.max(0.0, fade))

	if self.stateTime >= DEATH_DURATION and #self.blasts == 0 then
		self.state = "dead"
		Entity.queueDestroy(entity)
	end
end

function Boss:SpawnBlast()
	local x, y
	if math.random() < 0.7 then
		local pos = Transform.getPosition(self.entity)
		x = pos.x + math.random() * BODY_W * SCALE * 0.8 + BODY_W * SCALE * 0.1
		y = pos.y + math.random() * BODY_H * SCALE * 0.9
	else
		local segment = self.tail[math.random(#self.tail)]
		local pos = Transform.getPosition(segment.entity)
		x, y = pos.x, pos.y
	end

	local blast = Entity.create()
	Entity.setName(blast, "DobkeratopsExplosion")
	local half = EXPLOSION_SIZE * EXPLOSION_SCALE * 0.5
	Transform.setPosition(blast, x - half, y - half, 3)
	Component.addSprite(blast, EXPLOSION[1], 0, 0, EXPLOSION_SCALE, EXPLOSION_SCALE)
	replicate(blast, true)
	table.insert(self.blasts, { entity = blast, elapsed = 0.0, frame = 1 })
end

-- Every entity this script created that belongs to the boss.
function Boss:Parts()
	local parts = {}
	if self.core ~= nil then
		table.insert(parts, self.core)
	end
	for _, box in ipairs(self.boxes or {}) do
		table.insert(parts, box)
	end
	for _, segment in ipairs(self.tail or {}) do
		table.insert(parts, segment.entity)
	end
	return parts
end

function Boss:onDestroy(entity)
	for _, part in ipairs(self:Parts()) do
		Entity.queueDestroy(part)
	end
	for _, blast in ipairs(self.blasts or {}) do
		Entity.queueDestroy(blast.entity)
	end
	self.core = nil
	self.boxes = nil
	self.tail = nil
	self.blasts = {}
end

return Boss
