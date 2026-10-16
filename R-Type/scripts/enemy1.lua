local Enemy = {}

local SPRITES = {
	"assets/enemies/red-spinner/enemy1000.png",
	"assets/enemies/red-spinner/enemy1001.png",
	"assets/enemies/red-spinner/enemy1002.png",
	"assets/enemies/red-spinner/enemy1003.png",
	"assets/enemies/red-spinner/enemy1004.png",
	"assets/enemies/red-spinner/enemy1005.png",
	"assets/enemies/red-spinner/enemy1006.png",
	"assets/enemies/red-spinner/enemy1007.png",
}

local EXPLOSION = {
	"assets/effects/enemy-explosion-1000.png",
	"assets/effects/enemy-explosion-1001.png",
	"assets/effects/enemy-explosion-1002.png",
	"assets/effects/enemy-explosion-1003.png",
	"assets/effects/enemy-explosion-1004.png",
	"assets/effects/enemy-explosion-1005.png",
}

local FRAME_DURATION = 0.1

function Enemy:onCreate(entity)
	self.entity = entity
	self.frame = 1
	self.elapsed = 0.0
	self.exploding = false

	Component.setSpritePath(entity, SPRITES[self.frame])
	Component.addBoxCollider(entity, 32, 32)
end

function Enemy:onUpdate(entity, dt)
	if not dt or dt <= 0 then
		dt = 1.0 / 60.0
	end

	if self.exploding then
		self.elapsed = self.elapsed + dt

		while self.elapsed >= FRAME_DURATION do
			self.elapsed = self.elapsed - FRAME_DURATION
			self.frame = self.frame + 1

			if self.frame > #EXPLOSION then
				Entity.queueDestroy(entity)
				return
			end

			Component.setSpritePath(entity, EXPLOSION[self.frame])
		end

		return
	end

	self.elapsed = self.elapsed + dt

	while self.elapsed >= FRAME_DURATION do
		self.elapsed = self.elapsed - FRAME_DURATION
		self.frame = (self.frame % #SPRITES) + 1
		Component.setSpritePath(entity, SPRITES[self.frame])
	end
end

function Enemy:onCollisionEnter(this, other)
	if self.exploding then
		return
	end

	self.exploding = true
	self.frame = 1
	self.elapsed = 0.0

	Component.setSpritePath(this, EXPLOSION[self.frame])
end

return Enemy
