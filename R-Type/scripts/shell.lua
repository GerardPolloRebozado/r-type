local Shell = {}

local FRAME_DIR = "assets/enemies/shell/"

local ARMOUR_FIRST, ARMOUR_COUNT = 0, 16
local CORE_FIRST, CORE_COUNT = 16, 16
local FRAME0_ANGLE = -90.0
local FRAME_DIRECTION = 1
local GAP = 2

local RADIUS = 350.0
local ROT_SPEED = 0.5
local CORE_HP = 5
local ARMOUR_HP = 3

local SPEED_X = 0.0
local SCREEN_MARGIN = 400.0

local BODY_LAYER = "Enemy"
local BODY_MASK = Layers.bit("Player") + Layers.bit("PlayerBullet")
local KILLER_LAYER = "PlayerBullet"

local EXPLOSION = {
	"assets/effects/enemy-explosion-1000.png",
	"assets/effects/enemy-explosion-1001.png",
	"assets/effects/enemy-explosion-1002.png",
	"assets/effects/enemy-explosion-1003.png",
	"assets/effects/enemy-explosion-1004.png",
	"assets/effects/enemy-explosion-1005.png",
}
local EXPLOSION_FRAME_DURATION = 0.1
local CORPSE_SPRITE = "assets/enemies/shell/shell032.png"

local FIRE_INTERVAL = 4.0
local BULLET_SCRIPT = "scripts/shellBullet.lua"
local BULLET_SPRITE = "assets/enemies/shell/bullets/bullet000.png"
local BULLET_SCALE = 4.0
local BULLET_SIZE = 15.0
local PIECE_SIZE = 128.0

local function isNetworked()
	return Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked()
end

local function shouldSimulate(entity)
	if not isNetworked() then
		return true
	end
	if Net.isServer() then
		return true
	end
	return Net.networkId(entity) == nil
end

local function framePath(first, count, index)
	return string.format("%sshell%03d.png", FRAME_DIR, first + (index % count))
end

function Shell:onCreate(entity)
	self.entity = entity
	self.angle = 0.0
	self.inside = 0
	self.dead = false
	self.segments = {}
	self.coreHp = CORE_HP

	local start = Transform.getPosition(entity)
	self.x = start.x
	self.y = start.y

	Component.addAudioSource(entity, "assets/audio/explosion.mp3", false, false, 1)
	if isNetworked() then
		Net.replicate(entity, "Server", true, true)
	end

	if not shouldSimulate(entity) then
		return
	end

	local children = {}
	for _, child in ipairs(Component.getChildren(entity)) do
		if string.sub(Entity.getName(child), 1, 8) == "ShellSeg" then
			children[#children + 1] = child
		end
	end

	self.step = (2.0 * math.pi) / #children
	local keep = #children - GAP
	for i = keep + 1, #children do
		Entity.queueDestroy(children[i])
	end

	self.coreIndex = math.random(1, keep) -- the weak spot
	for i = 1, keep do
		local seg = children[i]
		Component.setParent(seg, entity, false)
		local isCore = (i == self.coreIndex)
		Entity.setName(seg, isCore and "ShellCore" or ("ShellArmour" .. i))

		local first, count = ARMOUR_FIRST, ARMOUR_COUNT
		if isCore then
			first, count = CORE_FIRST, CORE_COUNT
		end
		Component.setSpritePath(seg, framePath(first, count, i))
		Component.setColliderTrigger(seg, true)
		Component.setColliderLayer(seg, BODY_LAYER)
		Component.setColliderMask(seg, BODY_MASK)
		Component.connect(seg, "onCollisionEnter", entity, "onCollisionEnter")

		if isNetworked() then
			Net.replicate(seg, "Server", true, true)
		end

		self.segments[i] = {
			entity = seg,
			isCore = isCore,
			first = first,
			count = count,
			slot = i,
			hp = ARMOUR_HP,
			fireTimer = math.random() * FIRE_INTERVAL,
		}
	end
	self:PlaceSegments()
end

-- Segment use angle + i * (2pi / number of segments), around the pivot
function Shell:PlaceSegments()
	local step = self.step
	for _, seg in ipairs(self.segments) do
		local a = self.angle + (seg.slot - 1) * step
		Transform.setPosition(seg.entity, self.x + math.cos(a) * RADIUS, self.y + math.sin(a) * RADIUS)
		if seg.dead then
			goto continue
		end
		local deg = math.deg(a) * FRAME_DIRECTION + FRAME0_ANGLE
		local idx = math.floor(deg / (360.0 / seg.count) + 0.5) % seg.count
		if idx ~= seg.frameIdx then
			seg.frameIdx = idx
			Component.setSpritePath(seg.entity, framePath(seg.first, seg.count, idx))
		end
		::continue::
	end
end

function Shell:onUpdate(entity, dt)
	if not shouldSimulate(entity) then
		return
	end
	if not dt or dt <= 0 then
		dt = 1.0 / 60.0
	end

	self:AnimateDeaths(dt)

	self.angle = self.angle + ROT_SPEED * dt
	self.x = self.x - SPEED_X * dt
	Transform.setPosition(entity, self.x, self.y)

	self:PlaceSegments()
	if not self.dead and self.inside > 0 then
		for _, seg in ipairs(self.segments) do
			if not seg.dead then
				seg.fireTimer = seg.fireTimer - dt
				if seg.fireTimer <= 0.0 then
					seg.fireTimer = seg.fireTimer + FIRE_INTERVAL
					self:Fire(seg)
				end
			end
		end
	end

	if self.x < -SCREEN_MARGIN then
		self:DestroyAll(entity)
	end
end

function Shell:onZoneEnter(this, other)
	if Component.getColliderLayerName(other) == "Player" then
		self.inside = self.inside + 1
	end
end

function Shell:onZoneExit(this, other)
	if Component.getColliderLayerName(other) == "Player" then
		self.inside = math.max(self.inside - 1, 0)
	end
end

function Shell:onCollisionEnter(this, other)
	if not shouldSimulate(this) or self.dead then
		return
	end
	if Component.getColliderLayerName(other) ~= KILLER_LAYER then
		return
	end

	local seg = nil
	for _, s in ipairs(self.segments) do
		if s.entity == this then
			seg = s
			break
		end
	end
	if seg == nil or seg.dead then
		return
	end

	if not seg.isCore then
		seg.hp = seg.hp - 1
		if seg.hp <= 0 then
			self:BreakPiece(seg)
		end
		return
	end

	self.coreHp = self.coreHp - 1
	if self.coreHp > 0 then
		return
	end

	self.dead = true
	Component.playAudio(self.entity)
	for _, s in ipairs(self.segments) do
		if not s.dead then
			self:BreakPiece(s)
		end
	end
end

function Shell:BreakPiece(seg)
	seg.dead = true
	seg.deathFrame = 1
	seg.deathElapsed = 0.0
	Component.setColliderEnabled(seg.entity, false)
	Component.setSpritePath(seg.entity, EXPLOSION[1])
end

function Shell:AnimateDeaths(dt)
	for _, seg in ipairs(self.segments) do
		if seg.dead and seg.deathFrame ~= nil then
			seg.deathElapsed = seg.deathElapsed + dt
			while seg.deathElapsed >= EXPLOSION_FRAME_DURATION and seg.deathFrame ~= nil do
				seg.deathElapsed = seg.deathElapsed - EXPLOSION_FRAME_DURATION
				seg.deathFrame = seg.deathFrame + 1
				if seg.deathFrame > #EXPLOSION then
					seg.deathFrame = nil
					Component.setSpritePath(seg.entity, CORPSE_SPRITE)
				else
					Component.setSpritePath(seg.entity, EXPLOSION[seg.deathFrame])
				end
			end
		end
	end
end

function Shell:Fire(seg)
	local pos = Transform.getPosition(seg.entity)
	local cx = pos.x + PIECE_SIZE * 0.5
	local cy = pos.y + PIECE_SIZE * 0.5

	local bullet = Entity.create()
	Entity.setName(bullet, "ShellBullet")
	Transform.setPosition(bullet, cx - BULLET_SIZE * 0.5, cy - BULLET_SIZE * 0.5, 2)
	local shiftX = -(7 * BULLET_SCALE - BULLET_SIZE) * 0.5
	local shiftY = -(6 * BULLET_SCALE - BULLET_SIZE) * 0.5
	Component.addSprite(bullet, BULLET_SPRITE, shiftX, shiftY, BULLET_SCALE, BULLET_SCALE)
	Component.addBoxCollider(bullet, BULLET_SIZE, BULLET_SIZE)
	Component.setColliderLayer(bullet, "EnemyBullet")
	Component.setColliderMask(bullet, Layers.bit("Player"))
	Component.addScript(bullet, BULLET_SCRIPT, true)
	if isNetworked() then
		Net.replicate(bullet, "Server", true, true)
	end
end

function Shell:DestroyAll(entity)
	for _, s in ipairs(self.segments) do
		Entity.queueDestroy(s.entity)
	end
	self.segments = {}
	Entity.queueDestroy(entity)
end

function Shell:onDestroy(entity)
	log.info("[" .. tostring(entity) .. "] Shell destroyed")
end

return Shell
