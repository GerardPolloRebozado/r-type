local Player = {}

Player.speed = 500.0

local IDLE = "assets/player/sprite_003.png"
local DOWN_START = "assets/player/sprite_002.png"
local DOWN_MOVE = "assets/player/sprite_001.png"
local UP_START = "assets/player/sprite_004.png"
local UP_MOVE = "assets/player/sprite_005.png"

local START_DURATION = 0.2

function Player:onCreate(entity)
	self.entity = entity
	self.animDirection = nil
	self.animElapsed = 0.0
	self.currentSprite = nil

	Component.setSpritePath(entity, IDLE)
	self.currentSprite = IDLE
end

function Player:onUpdate(entity, dt)
	if not dt or dt <= 0 then
		dt = 1.0 / 60.0
	end

	local up = Controller.isActionActive("UP")
	local down = Controller.isActionActive("DOWN")

	local direction = nil
	if up and not down then
		direction = "up"
	elseif down and not up then
		direction = "down"
	end

	if direction ~= self.animDirection then
		self.animDirection = direction
		self.animElapsed = 0.0
	end

	local sprite = IDLE
	if direction ~= nil then
		self.animElapsed = self.animElapsed + dt
		if self.animElapsed < START_DURATION then
			sprite = (direction == "up") and UP_START or DOWN_START
		else
			sprite = (direction == "up") and UP_MOVE or DOWN_MOVE
		end
	end

	if sprite ~= self.currentSprite then
		Component.setSpritePath(entity, sprite)
		self.currentSprite = sprite
	end

	local dx, dy = 0.0, 0.0

	if Controller.isActionActive("LEFT") then
		dx = dx - 1.0
	end
	if Controller.isActionActive("RIGHT") then
		dx = dx + 1.0
	end
	if up then
		dy = dy - 1.0
	end
	if down then
		dy = dy + 1.0
	end

	if dx ~= 0.0 or dy ~= 0.0 then
		local len = math.sqrt(dx * dx + dy * dy)
		Transform.translate(entity, (dx / len) * self.speed * dt, (dy / len) * self.speed * dt)
	end
end

function Player:collisionTest(one, two)
	log.info("Connection!")
end

function Player:onDestroy(entity)
	log.info("[player.lua] onDestroy entity " .. tostring(entity))
end

return Player
