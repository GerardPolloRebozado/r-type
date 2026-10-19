-- playerGun.lua
-- Attached to: Gun (child of Player entity)
-- Features: Muzzle flash + server-authoritative projectiles.
--
-- Only the single-player process and the server fire. A networked client
-- sends its SHOOT bit in the input packet; the server spawns the projectile and
-- marks it as a replicated runtime entity, so every client sees it.

local Script = {}

local MUZZLE_01 = "assets/player/shoot/muzzleflash_000.png"
local MUZZLE_02 = "assets/player/shoot/muzzleflash_001.png"
local FRAME_DURATION = 0.05

local PROJECTILE_SPRITE = "assets/player/shoot/laserbeam.png"

local PROJECTILE_SPEED = 1800.0 -- pixels per second
local PROJECTILE_LIFETIME = 2.0 -- seconds before auto-remove
local FIRE_COOLDOWN = 0.15 -- seconds between shots

local function isNetworked()
    return Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked()
end

local function shouldSimulate()
    if not isNetworked() then
        return true
    end
    return Net.isServer()
end

local function readInput(owner)
    if Net ~= nil and Net.input ~= nil then
        local input = Net.input(owner)
        if input ~= nil then
            return input
        end
    end
    local input = {}
    if Controller.isActionActive("SHOOT") then input.SHOOT = true end
    return input
end

local function isShootDown(entity)
    local owner = Component.getParent(entity)
    if owner == nil then
        return false
    end
    return readInput(owner).SHOOT == true
end

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
end

function Script:onUpdate(entity, dt)
    -- Networked client: the server decides when bullets exist.
    if not shouldSimulate() then
        return
    end

    if not dt or dt <= 0 then
        dt = 1.0 / 60.0
    end

    local shootDown = isShootDown(entity)

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
        self.cooldown = FIRE_COOLDOWN
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

    Component.setColliderLayer(projEntity, "PlayerBullet")
    Component.setColliderMask(projEntity, Layers.bit("Enemy") + Layers.bit("EnemyBullet") + Layers.bit("Wall"))

    Component.setOpacity(projEntity, 1.0)

    Component.connect(projEntity, "onCollisionEnter", self.entity, "onProjectileHit")

    -- In multiplayer this assigns a network id and makes ReplicationSystem
    -- announce the spawn + sync its position/sprite to every client.
    if isNetworked() then
        Net.replicate(projEntity, "Server", true, true)
    end

    table.insert(self.projectiles, {
        entity = projEntity,
        timer = PROJECTILE_LIFETIME,
    })
end

function Script:onProjectileHit(bullet, other)
    Entity.queueDestroy(bullet)
    for i = #self.projectiles, 1, -1 do
        if self.projectiles[i].entity == bullet then
            table.remove(self.projectiles, i)
            break
        end
    end
end

function Script:onDestroy(entity)
    for _, proj in ipairs(self.projectiles) do
        Entity.destroy(proj.entity)
    end
    self.projectiles = {}
end

return Script
