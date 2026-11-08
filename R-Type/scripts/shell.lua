local Shell = {}

local FRAME_DIR = "assets/enemies/shell/"

local ARMOUR_FIRST, ARMOUR_COUNT = 0, 16
local CORE_FIRST, CORE_COUNT = 16, 16
local GAP = 2

local RADIUS = 250.0
local ROT_SPEED = 1.0
local CORE_HP = 5

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
	self.time = 0.0
	self.angle = 0.0
	self.exploding = false
	self.frame = 1
	self.elapsed = 0.0
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
		children[#children + 1] = child
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

		Component.setColliderLayer(seg, BODY_LAYER)
		Component.setColliderMask(seg, BODY_MASK)
		Component.connect(seg, "onCollisionEnter", entity, "onCollisionEnter")

		if isNetworked() then
			Net.replicate(seg, "Server", true, true)
		end

		self.segments[i] = { entity = seg, isCore = isCore, first = first, count = count }
	end
	self:PlaceSegments()
end

-- Segment use angle + i * (2pi / number of segments), around the pivot
function Shell:PlaceSegments()
	local step = self.step
	for i, seg in ipairs(self.segments) do
		local a = self.angle + (i - 1) * step
		Transform.setPosition(seg.entity, self.x + math.cos(a) * RADIUS, self.y + math.sin(a) * RADIUS)
	end
end

function Shell:onUpdate(entity, dt)
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
	self.angle = self.angle + ROT_SPEED * dt
	self.x = self.x - SPEED_X * dt
	Transform.setPosition(entity, self.x, self.y)

	self:PlaceSegments()

	if self.x < -SCREEN_MARGIN then
		self:DestroyAll(entity)
	end
end

function Shell:onCollisionEnter(this, other)
	if not shouldSimulate(this) or self.exploding then
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
	if seg == nil or not seg.isCore then
		return
	end

	self.coreHp = self.coreHp - 1
	if self.coreHp > 0 then
		return
	end

	self.exploding = true
	self.frame = 1
	self.elapsed = 0.0
	Component.playAudio(self.entity)
	for _, s in ipairs(self.segments) do
		Component.setSpritePath(s.entity, EXPLOSION[1])
	end
end

function Shell:AnimateExplosion(entity, dt)
	self.elapsed = self.elapsed + dt
	while self.elapsed >= EXPLOSION_FRAME_DURATION do
		self.elapsed = self.elapsed - EXPLOSION_FRAME_DURATION
		self.frame = self.frame + 1
		if self.frame > #EXPLOSION then
			self:DestroyAll(entity)
			return
		end
		for _, s in ipairs(self.segments) do
			Component.setSpritePath(s.entity, EXPLOSION[self.frame])
		end
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
