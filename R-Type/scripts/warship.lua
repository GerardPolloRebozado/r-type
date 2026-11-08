-- warship.lua
-- Attached to: Warship (the hull, root of Warship.scene)
--
-- The look of the ship is built in the prefab: every part is a child entity
-- of the hull. This script turns those children into breakable parts:
--   - each part gets an invisible hitbox around its visible pixels;
--   - each part takes MAX_HEALTH player shots, then explodes;
--   - once every part is gone the whole warship explodes.
--
-- Movement: the warship slides in from the right of the screen, then flies
-- along with the scrolling camera, drifting slowly up and down. Only the
-- hull is moved: parts and hitboxes are relative children and follow it.
-- Its size comes from the hull's sprite scale (set in the prefab).
--
-- Touching the warship is harmless: hitboxes are on the Enemy layer (so the
-- super shot hits them) but their mask only accepts player shots, and two
-- colliders only touch when both masks accept each other. What hurts the
-- player are the shots fired by the warship: they go on the EnemyBullet
-- layer, which player.lua already handles.
local Warship = {}

local DIR = "assets/enemies/warship/"

local MAX_HEALTH = 5 -- player shots needed to break one part

local HITBOX_LAYER = "Enemy"
local HITBOX_MASK = Layers.bit("PlayerBullet")
local KILLER_LAYER = "PlayerBullet"

-- Visible area of each part sprite, { x, y, width, height } in sheet pixels
-- from the sprite's top-left (the pngs have transparent padding).
local VISIBLE = {
	["bulb-bottom000"] = { 21, 18, 32, 15 },
	["bulb-bottom001"] = { 21, 18, 30, 20 },
	["bulb-bottom004"] = { 19, 21, 17, 28 },
	["bulb-top002"] = { 22, 13, 26, 24 },
	["cannon"] = { 0, 0, 96, 32 },
	["dome005"] = { 0, 0, 32, 27 },
	["engine000"] = { 1, 0, 64, 32 },
	["hatch000"] = { 0, 0, 55, 32 },
	["hatch001"] = { 0, 0, 64, 32 },
	["mine000"] = { 3, 2, 25, 27 },
	["mine004"] = { 2, 3, 28, 25 },
	["mine008"] = { 3, 2, 25, 27 },
	["mine015"] = { 3, 2, 25, 27 },
	["thruster000"] = { 4, 2, 24, 24 },
	["turret-bottom000"] = { 0, 0, 28, 13 },
	["turret-bottom001"] = { 5, 0, 23, 15 },
	["turret-bottom002"] = { 10, 0, 18, 18 },
	["turret-top001"] = { 5, 8, 23, 15 },
	["turret-top003"] = { 12, 2, 16, 21 },
	["turret-top004"] = { 12, 0, 16, 23 },
	["turret-top005"] = { 12, 2, 16, 21 },
	["undercarriage"] = { 0, 0, 160, 32 },
}

-- Children that are not parts but belong to one: "Flame2" goes away with
-- "Thruster2".
local ATTACHED_TO = {
	Flame = "Thruster",
}

local HIT_FLASH = 0.08 -- seconds a part stays see-through after a hit
local HIT_ALPHA = 0.4

-- Shooting ---------------------------------------------------------------
-- Turrets turn towards the closest player and fire at it. Each family only
-- covers half a circle: "top" turrets aim between right and left over the
-- top, "bottom" ones under the bottom. A player outside that range gets
-- shot at from the closest edge. Frame k of a family points at k * 22.5
-- degrees (see assets/enemies/warship/separator.py).
-- origin: where shots come out, in sheet pixels from the sprite's top-left.
local SHOOTERS = {
	["turret-top"] = { half = "top", bullet = "orb", origin = { 20, 14 } },
	["turret-bottom"] = { half = "bottom", bullet = "orb", origin = { 20, 10 } },
	["dome"] = { half = "top", bullet = "fireball-small", origin = { 16, 14 } },
	["bulb-top"] = { half = "top", bullet = "fireball", origin = { 28, 28 } },
	["bulb-bottom"] = { half = "bottom", bullet = "fireball", origin = { 28, 28 } },
}
local BARREL = 10 -- sheet pixels from the origin to the end of the barrel

-- size: sprite canvas, hit: collider (both in sheet pixels, scaled like the
-- warship); speed in pixels per second.
local BULLETS = {
	orb = {
		frames = {
			DIR .. "warship-orb-small000.png",
			DIR .. "warship-orb-small001.png",
			DIR .. "warship-orb-small002.png",
			DIR .. "warship-orb-small003.png",
		},
		frameDuration = 0.08,
		size = 12, hit = 8, speed = 380,
	},
	-- One png per direction: "fireball%03d" with 16 directions.
	["fireball-small"] = { directional = DIR .. "warship-fireball-small%03d.png", size = 32, hit = 10, speed = 430 },
	fireball = { directional = DIR .. "warship-fireball%03d.png", size = 32, hit = 14, speed = 360 },
}
local BULLET_SCRIPT = "scripts/warshipBullet.lua"
local BULLET_Z = 5

local FIRE_INTERVAL = 0.5 -- one random turret fires this often
local AIM_INTERVAL = 0.12 -- how often turrets turn towards the player

-- The front cannon charges (its vents flash) then fires a beam to the left.
local CANNON_INTERVAL = 5.0
local CANNON_CHARGE = 0.6
local CANNON_NOZZLE = { 0, 16 } -- sheet pixels from the cannon's top-left
local CANNON_VENTS = { -- flash sprites around the nozzle
	{ DIR .. "warship-cannon-flash000.png", -4, -41 },
	{ DIR .. "warship-cannon-flash001.png", -4, 41 },
}
local BEAM = { sprite = DIR .. "warship-beam000.png", width = 130, height = 32, hit = 20, speed = 1100 }

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
local EXPLOSION_SCALE = 1.5 -- relative to the warship's scale
local EXPLOSION_SOUND = "assets/audio/explosion.mp3"

-- Size of the whole ship in sheet pixels, from the hull's top-left: the
-- cannon sticks out 50px on the left and the flames hang below the hull.
local SHIP_LEFT, SHIP_WIDTH, SHIP_HEIGHT = -50, 642, 258
local HULL_W, HULL_H = 592, 176

-- Movement, in screen pixels.
local ENTER_DURATION = 4.0 -- slide in from the right
local SCREEN_MARGIN_RIGHT = 60 -- gap between the rear of the ship and the screen edge
local DRIFT_HEIGHT = 40 -- up/down drift around the middle of the screen
local DRIFT_PERIOD = 6.0 -- seconds for one full up-and-down

local HULL_DESTROYED = DIR .. "warship-hull-destroyed.png"
local DEATH_DURATION = 2.5
local DEATH_BLAST_INTERVAL = 0.08

-- Only the server (and single-player) simulates replicated enemies.
local function shouldSimulate(entity)
	if Net == nil or not Net.isNetworked() then
		return true
	end
	if Net.isServer() then
		return true
	end
	return Net.networkId(entity) == nil
end

local function viewport()
	if Camera ~= nil and Camera.viewport ~= nil then
		return Camera.viewport()
	end
	return { x = 0, y = 0, width = 1920, height = 1080 }
end

local function isNetworked()
	return Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked()
end

local function replicate(entity)
	if isNetworked() then
		Net.replicate(entity, "Server", true, true)
	end
end

local atan2 = math.atan2 or math.atan
local STEP = math.pi / 8 -- 22.5 degrees between two frames

-- Centers of every active player, as { x, y } pairs.
local function playerCenters()
	local centers = {}
	for _, entity in ipairs(Component.entitiesWith("Collider")) do
		if Component.getColliderLayerName(entity) == "Player" and Component.getColliderEnabled(entity) then
			local pos = Transform.getPosition(entity)
			local size = Component.getColliderSize(entity)
			table.insert(centers, { pos.x + size.w * 0.5, pos.y + size.h * 0.5 })
		end
	end
	return centers
end

-- Closest of those centers to (x, y), or nil.
local function closest(centers, x, y)
	local bestX, bestY, bestDist = nil, nil, math.huge
	for _, c in ipairs(centers) do
		local dist = (c[1] - x) * (c[1] - x) + (c[2] - y) * (c[2] - y)
		if dist < bestDist then
			bestX, bestY, bestDist = c[1], c[2], dist
		end
	end
	return bestX, bestY
end

-- Angle (radians, counter-clockwise from the right, y up) from a turret to
-- its target, kept inside the half circle the turret covers.
local function turretAngle(half, x, y, tx, ty)
	local a = atan2(-(ty - y), tx - x)
	if a < 0 then
		a = a + 2 * math.pi
	end
	if half == "top" and a > math.pi then
		a = (a < 1.5 * math.pi) and math.pi or 0.0
	elseif half == "bottom" and a < math.pi and a > 0 then
		a = (a > 0.5 * math.pi) and math.pi or 2 * math.pi
	end
	return a
end

-- Frame of a turret family for an angle: top sets go 000 (right) -> 008
-- (left) over the top, bottom sets go 000 (right) -> 008 (left) underneath.
local function turretFrame(half, a)
	if half == "top" then
		return math.floor(a / STEP + 0.5)
	end
	if a == 0 then
		return 0
	end
	return math.floor((2 * math.pi - a) / STEP + 0.5)
end

-- "assets/enemies/warship/warship-turret-top001.png" -> "turret-top001"
local function spriteKey(entity)
	local path = Component.getSpritePath(entity)
	if path == nil then
		return nil
	end
	return path:match("warship%-([%w%-]+)%.png$")
end

-- "Thruster2" -> "Thruster", "2"
local function splitName(name)
	return name:match("^(.-)(%d*)$")
end

function Warship:onCreate(entity)
	self.entity = entity
	self.state = "fight"
	self.parts = {} -- part entity -> part
	self.byHitbox = {} -- hitbox entity -> part
	self.partCount = 0
	self.spent = {} -- player shots that already hit a part this frame
	self.blasts = {}
	self.blastTimer = 0.0
	self.stateTime = 0.0
	self.time = 0.0
	self.ready = false
	self.fireTimer = FIRE_INTERVAL
	self.aimTimer = 0.0
	self.cannonTimer = CANNON_INTERVAL
	self.charge = nil -- { timer, vents } while the cannon charges

	local scale = Component.getSpriteScale(entity)
	self.scale = scale and scale.x or 2.0

	Component.addAudioSource(entity, EXPLOSION_SOUND, false, false, 1)
end

-- Where the hull goes on screen: rear near the right edge, ship centered
-- vertically. The start is just past the right edge.
function Warship:SetupMovement()
	local vp = viewport()
	local s = self.scale
	self.targetScreenX = vp.width - SCREEN_MARGIN_RIGHT - HULL_W * s
	self.startScreenX = vp.width - SHIP_LEFT * s
	self.screenX = self.startScreenX
	self.baseScreenY = (vp.height - SHIP_HEIGHT * s) * 0.5
	self.enterTime = 0.0
	self:PlaceOnScreen(0.0)
end

function Warship:PlaceOnScreen(drift)
	local vp = viewport()
	local pos = Transform.getPosition(self.entity)
	Transform.setPosition(self.entity, vp.x + self.screenX, vp.y + self.baseScreenY + drift, pos.z)
end

function Warship:UpdateMovement(dt)
	if self.enterTime < ENTER_DURATION then
		self.enterTime = math.min(ENTER_DURATION, self.enterTime + dt)
		local t = self.enterTime / ENTER_DURATION
		local ease = 1.0 - (1.0 - t) * (1.0 - t)
		self.screenX = self.startScreenX + (self.targetScreenX - self.startScreenX) * ease
	end
	local drift = math.sin(self.time * 2.0 * math.pi / DRIFT_PERIOD) * DRIFT_HEIGHT
	self:PlaceOnScreen(drift)
end

-- Scene children may not exist yet in onCreate: collect them on the first
-- update.
function Warship:CollectParts(entity)
	self.ready = true
	local children = Component.getChildren(entity)
	local attached = {}

	for _, child in ipairs(children) do
		local name = Entity.getName(child)
		local base, index = splitName(name)
		if ATTACHED_TO[base] ~= nil then
			attached[child] = ATTACHED_TO[base] .. index
		else
			self:MakePart(child, name)
		end
	end

	-- Hook flames & co. to the part they belong to.
	for child, ownerName in pairs(attached) do
		for _, part in pairs(self.parts) do
			if part.name == ownerName then
				table.insert(part.attached, child)
			end
		end
	end

	log.info(string.format("[warship.lua] %d parts to break", self.partCount))
end

function Warship:MakePart(child, name)
	local key = spriteKey(child)
	local box = key and VISIBLE[key]
	if box == nil then
		log.error(string.format("[warship.lua] no hitbox for %s (%s)", name, tostring(key)))
		return
	end

	local scale = Component.getSpriteScale(child)
	local pos = Transform.getPosition(child)
	local hitbox = Entity.create()
	Entity.setName(hitbox, name .. "Hitbox")
	Transform.setPosition(hitbox, pos.x + box[1] * scale.x, pos.y + box[2] * scale.y, pos.z)
	Component.addBoxCollider(hitbox, box[3] * scale.x, box[4] * scale.y, true)
	Component.setColliderLayer(hitbox, HITBOX_LAYER)
	Component.setColliderMask(hitbox, HITBOX_MASK)
	Component.setParent(hitbox, child, true)
	Component.connect(hitbox, "onCollisionEnter", self.entity, "onPartHit")

	local family = key:match("^(.-)%d+$")
	local part = {
		entity = child,
		hitbox = hitbox,
		name = name,
		health = MAX_HEALTH,
		flash = 0.0,
		attached = {},
		family = family,
		shooter = family and SHOOTERS[family],
		frame = tonumber(key:match("(%d+)$")),
		isCannon = key == "cannon",
	}
	self.parts[child] = part
	self.byHitbox[hitbox] = part
	self.partCount = self.partCount + 1
end

function Warship:onUpdate(entity, dt)
	if not shouldSimulate(entity) then
		return
	end
	if not dt or dt <= 0 then
		dt = 1.0 / 60.0
	end
	-- Hitboxes are built where the parts are, before the ship moves: from
	-- then on they follow the hull like the parts do.
	if not self.ready then
		self:CollectParts(entity)
		self:SetupMovement()
	end

	self.spent = {}
	self.stateTime = self.stateTime + dt
	self.time = self.time + dt

	if self.state ~= "dead" then
		self:UpdateMovement(dt)
	end
	-- Shoot once fully on screen.
	if self.state == "fight" and self.enterTime >= ENTER_DURATION then
		self:UpdateShooting(dt)
	end

	for _, part in pairs(self.parts) do
		if part.flash > 0.0 then
			part.flash = part.flash - dt
			if part.flash <= 0.0 then
				Component.setOpacity(part.entity, 1.0)
			end
		end
	end

	if self.state == "dying" then
		self:UpdateDying(entity, dt)
	end
	self:UpdateBlasts(dt)
end

function Warship:UpdateShooting(dt)
	self.aimTimer = self.aimTimer - dt
	if self.aimTimer <= 0.0 then
		self.aimTimer = AIM_INTERVAL
		self:AimTurrets()
	end

	self.fireTimer = self.fireTimer - dt
	if self.fireTimer <= 0.0 then
		self.fireTimer = FIRE_INTERVAL
		local shooters = {}
		for _, part in pairs(self.parts) do
			if part.shooter ~= nil then
				table.insert(shooters, part)
			end
		end
		if #shooters > 0 then
			self:FireTurret(shooters[math.random(#shooters)])
		end
	end

	self:UpdateCannon(dt)
end

-- Where a turret's shots come out (before adding the barrel), in world space.
function Warship:TurretOrigin(part)
	local pos = Transform.getPosition(part.entity)
	local o = part.shooter.origin
	return pos.x + o[1] * self.scale, pos.y + o[2] * self.scale
end

-- Turns every turret towards the closest player.
function Warship:AimTurrets()
	local players = playerCenters()
	for _, part in pairs(self.parts) do
		local shooter = part.shooter
		if shooter ~= nil then
			local x, y = self:TurretOrigin(part)
			local tx, ty = closest(players, x, y)
			if tx ~= nil then
				local frame = turretFrame(shooter.half, turretAngle(shooter.half, x, y, tx, ty))
				if frame ~= part.frame then
					part.frame = frame
					Component.setSpritePath(part.entity,
						string.format("%swarship-%s%03d.png", DIR, part.family, frame))
				end
			end
		end
	end
end

function Warship:FireTurret(part)
	local shooter = part.shooter
	local x, y = self:TurretOrigin(part)
	local tx, ty = closest(playerCenters(), x, y)
	if tx == nil then
		return
	end
	local a = turretAngle(shooter.half, x, y, tx, ty)
	local dx, dy = math.cos(a), -math.sin(a)
	local reach = BARREL * self.scale
	local kind = BULLETS[shooter.bullet]

	local sprite, frames
	if kind.directional ~= nil then
		-- 16 pngs, one every 22.5 degrees, 000 pointing right.
		local index = math.floor(a / STEP + 0.5) % 16
		sprite = string.format(kind.directional, index)
	else
		frames = kind.frames
		sprite = frames[1]
	end
	self:SpawnBullet(x + dx * reach, y + dy * reach, sprite, kind.size, kind.size, kind.hit, kind.hit,
		dx * kind.speed, dy * kind.speed, frames, kind.frameDuration)
end

-- Bullet centered on (x, y). Sizes are in sheet pixels.
function Warship:SpawnBullet(x, y, sprite, spriteW, spriteH, hitW, hitH, vx, vy, frames, frameDuration)
	local s = self.scale
	local bullet = Entity.create()
	Entity.setName(bullet, "WarshipBullet")
	Transform.setPosition(bullet, x - hitW * s * 0.5, y - hitH * s * 0.5, BULLET_Z)
	-- The collider is smaller than the png: shift the sprite to keep both centered.
	Component.addSprite(bullet, sprite, -(spriteW - hitW) * s * 0.5, -(spriteH - hitH) * s * 0.5, s, s)
	Component.addBoxCollider(bullet, hitW * s, hitH * s)
	Component.setColliderLayer(bullet, "EnemyBullet")
	Component.setColliderMask(bullet, Layers.bit("Player"))
	-- Read (and cleared) by warshipBullet.lua when it starts.
	WarshipBulletSpecs = WarshipBulletSpecs or {}
	WarshipBulletSpecs[bullet] = { vx = vx, vy = vy, frames = frames, frameDuration = frameDuration }
	-- Own script so bullets keep flying after the warship dies.
	Component.addScript(bullet, BULLET_SCRIPT, true)
	replicate(bullet)
end

function Warship:FindCannon()
	for _, part in pairs(self.parts) do
		if part.isCannon then
			return part
		end
	end
	return nil
end

function Warship:UpdateCannon(dt)
	local cannon = self:FindCannon()
	if cannon == nil then
		self.charge = nil -- vents were children of the cannon: already gone
		return
	end

	if self.charge == nil then
		self.cannonTimer = self.cannonTimer - dt
		if self.cannonTimer <= 0.0 then
			self:StartCharge(cannon)
		end
		return
	end

	self.charge.timer = self.charge.timer - dt
	if self.charge.timer <= 0.0 then
		for _, vent in ipairs(self.charge.vents) do
			Entity.queueDestroy(vent)
		end
		self.charge = nil
		self.cannonTimer = CANNON_INTERVAL
		self:FireBeam(cannon)
	end
end

-- The vents around the nozzle flash while the cannon charges.
function Warship:StartCharge(cannon)
	local pos = Transform.getPosition(cannon.entity)
	local vents = {}
	for _, vent in ipairs(CANNON_VENTS) do
		local e = Entity.create()
		Entity.setName(e, "WarshipCannonVent")
		Transform.setPosition(e, pos.x + vent[2] * self.scale, pos.y + vent[3] * self.scale, pos.z + 1)
		Component.addSprite(e, vent[1], 0, 0, self.scale, self.scale)
		Component.setParent(e, cannon.entity, true)
		replicate(e)
		table.insert(vents, e)
	end
	self.charge = { timer = CANNON_CHARGE, vents = vents }
end

-- Beam flying left, its right end starting at the nozzle.
function Warship:FireBeam(cannon)
	local pos = Transform.getPosition(cannon.entity)
	local s = self.scale
	local nozzleX = pos.x + CANNON_NOZZLE[1] * s
	local nozzleY = pos.y + CANNON_NOZZLE[2] * s
	self:SpawnBullet(nozzleX - BEAM.width * s * 0.5, nozzleY, BEAM.sprite, BEAM.width, BEAM.height,
		BEAM.width, BEAM.hit, -BEAM.speed, 0.0)
end

function Warship:onPartHit(hitbox, other)
	if not shouldSimulate(self.entity) or self.state ~= "fight" then
		return
	end
	if Component.getColliderLayerName(other) ~= KILLER_LAYER then
		return
	end
	-- Parts overlap: one shot only damages the first part it touches.
	if self.spent[other] then
		return
	end
	local part = self.byHitbox[hitbox]
	if part == nil then
		return
	end
	self.spent[other] = true

	part.health = part.health - 1
	if part.health > 0 then
		part.flash = HIT_FLASH
		Component.setOpacity(part.entity, HIT_ALPHA)
		return
	end
	self:BreakPart(part)
end

function Warship:BreakPart(part)
	local pos = Transform.getPosition(part.hitbox)
	local size = Component.getColliderSize(part.hitbox)
	self:SpawnBlast(pos.x + size.w * 0.5, pos.y + size.h * 0.5)
	Component.playAudio(self.entity)

	self.parts[part.entity] = nil
	self.byHitbox[part.hitbox] = nil
	self.partCount = self.partCount - 1
	Entity.queueDestroy(part.entity) -- takes its hitbox (a child) with it
	for _, child in ipairs(part.attached) do
		Entity.queueDestroy(child)
	end

	log.info(string.format("[warship.lua] %s destroyed, %d parts left", part.name, self.partCount))
	if self.partCount <= 0 then
		self:StartDying()
	end
end

function Warship:StartDying()
	self.state = "dying"
	self.stateTime = 0.0
	self.blastTimer = 0.0
	Component.setSpritePath(self.entity, HULL_DESTROYED)
end

function Warship:UpdateDying(entity, dt)
	self.blastTimer = self.blastTimer - dt
	if self.stateTime < DEATH_DURATION and self.blastTimer <= 0.0 then
		self.blastTimer = DEATH_BLAST_INTERVAL
		local pos = Transform.getPosition(entity)
		self:SpawnBlast(pos.x + math.random() * HULL_W * self.scale,
			pos.y + math.random() * HULL_H * self.scale)
		if math.random() < 0.3 then
			Component.playAudio(entity)
		end
	end

	-- Fade out during the second half.
	local fade = 1.0 - math.max(0.0, (self.stateTime - DEATH_DURATION * 0.5) / (DEATH_DURATION * 0.5))
	Component.setOpacity(entity, math.max(0.0, fade))

	if self.stateTime >= DEATH_DURATION and #self.blasts == 0 then
		self.state = "dead"
		Entity.queueDestroy(entity)
	end
end

-- Explosion centered on (x, y).
function Warship:SpawnBlast(x, y)
	local blast = Entity.create()
	Entity.setName(blast, "WarshipExplosion")
	local scale = EXPLOSION_SCALE * self.scale
	local half = EXPLOSION_SIZE * scale * 0.5
	Transform.setPosition(blast, x - half, y - half, 10)
	Component.addSprite(blast, EXPLOSION[1], 0, 0, scale, scale)
	table.insert(self.blasts, { entity = blast, elapsed = 0.0, frame = 1 })
end

function Warship:UpdateBlasts(dt)
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
end

-- Parts and hitboxes are children of the hull: they go with it.
function Warship:onDestroy(entity)
	for _, blast in ipairs(self.blasts or {}) do
		Entity.queueDestroy(blast.entity)
	end
	self.parts = {}
	self.byHitbox = {}
	self.blasts = {}
end

return Warship
