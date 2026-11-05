-- playerGun.lua
-- Attached to: Gun (child of Player entity)
-- Features: Muzzle flash + server-authoritative projectiles.
--
-- Only the single-player process and the server fire. A networked client
-- sends its SHOOT bit in the input packet; the server spawns the projectile and
-- marks it as a replicated runtime entity, so every client sees it.

local PlayerGun = {}

local MUZZLE_01 = "assets/player/shoot/muzzleflash_000.png"
local MUZZLE_02 = "assets/player/shoot/muzzleflash_001.png"
local FRAME_DURATION = 0.05

local SHOOT_SPRITE = "assets/player/shoot/laserbeam.png"
local SUPER_SHOOT_SPRITE = "assets/player/shoot/megabullet1.png"

local PROJECTILE_SPEED = 1800.0 -- pixels per second
local PROJECTILE_LIFETIME = 2.0 -- seconds before auto-remove
local FIRE_COOLDOWN = 0.15 -- seconds between shots
local MAX_BULLETS = 24 -- cap active bullets to bound worst case
local SCREEN_MARGIN = 128.0

local function isNetworked()
    return Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked()
end

local function shouldSimulate()
    if not isNetworked() then
        return true
    end
    return Net.isServer()
end

function PlayerGun:readInput()
    local owner = Component.getParent(self.entity)
    if Player.active == false then
        return {}
    end
    local input = {
        SUPER_SHOOT = false,
        SHOOT = false,
    }
    local netInput = Net.input(owner)
    if netInput == nil then
        netInput = input
    end
    if Controller.isActionActive("SHOOT") or netInput.SHOOT then
        self.shootCounter = (self.shootCounter or 0) + 1
        if self.shootCounter >= 30 then
            input.SUPER_SHOOT = true
        end
    else
        if self.shootCounter and self.shootCounter > 0 then
            if self.shootCounter < 30 then
                input.SHOOT = true
            end
            self.shootCounter = 0
        end
    end
    return input
end

local function isOutsideScreen(entity)
    local position = Transform.getPosition(entity)
    if Camera == nil or Camera.viewport == nil then
        return false
    end

    local viewport = Camera.viewport()
    return position.x < viewport.x - SCREEN_MARGIN or
            position.x > viewport.x + viewport.width + SCREEN_MARGIN or
            position.y < viewport.y - SCREEN_MARGIN or
            position.y > viewport.y + viewport.height + SCREEN_MARGIN
end

local function destroyProjectiles(self)
    for _, projectile in ipairs(self.projectiles) do
        Entity.destroy(projectile.entity)
    end
    self.projectiles = {}
end

function PlayerGun:onCreate(entity)
    self.entity = entity
    self.wasShootDown = false
    self.firing = false
    self.phase = 0
    self.elapsed = 0.0

    self.shootCounter = 0
    self.shootDown = false
    self.cooldown = 0.0
    self.projectiles = {}

    Component.setSpritePath(entity, MUZZLE_01)
    Component.setOpacity(entity, 0.0)
end

function PlayerGun:onUpdate(entity, dt)
    -- Networked client: the server decides when bullets exist.
    if not shouldSimulate() then
        return
    end

    if not dt or dt <= 0 then
        dt = 1.0 / 60.0
    end

    local shootInput = self:readInput()
    local isShooting = shootInput.SHOOT == true or shootInput.SUPER_SHOOT

    if isShooting and not self.wasShootDown and not self.firing then
        self.firing = true
        self.phase = 1
        self.elapsed = 0.0

        Component.setSpritePath(entity, MUZZLE_01)
        Component.setOpacity(entity, 1.0)
    end

    self.wasShootDown = isShooting

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

    if isShooting and not self.shootDown and self.cooldown <= 0 and #self.projectiles < MAX_BULLETS then
        self.cooldown = FIRE_COOLDOWN
        if shootInput.SUPER_SHOOT then
            self:fireSuperProjectile()
        else
            self:fireProjectile()
        end
    end

    self.shootDown = isShooting

    for i = #self.projectiles, 1, -1 do
        local proj = self.projectiles[i]
        local projEntity = proj.entity

        Transform.translate(projEntity, PROJECTILE_SPEED * dt, 0)

        proj.timer = proj.timer - dt
        if proj.timer <= 0 or isOutsideScreen(projEntity) then
            Entity.destroy(projEntity)
            table.remove(self.projectiles, i)
        end
    end
end

function PlayerGun:fireProjectile()
    local projEntity = Entity.create()

    local gunPos = Transform.getPosition(self.entity)
    local gunPosX = gunPos.x
    local gunPosY = gunPos.y
    Transform.setPosition(projEntity, gunPosX, gunPosY + 6)
    Component.addSprite(projEntity, SHOOT_SPRITE, 0, 0, 4.0, 4.0)
    Component.addBoxCollider(projEntity, 32, 32)

    Component.setColliderLayer(projEntity, "PlayerBullet")
    Component.setColliderMask(projEntity, Layers.bit("Enemy") + Layers.bit("EnemyBullet") + Layers.bit("Wall"))

    Component.setOpacity(projEntity, 1.0)

    Component.connect(projEntity, "onCollisionEnter", self.entity, "onProjectileHit")

    -- In multiplayer this assigns a network id and makes ReplicationSystem
    -- announce the spawn + sync its position/sprite to every client.
    if isNetworked() then
        Net.replicate(projEntity, "Server", true, false)
    end

    table.insert(self.projectiles, {
        entity = projEntity,
        timer = PROJECTILE_LIFETIME,
    })
end

function PlayerGun:fireSuperProjectile()
    local projEntity = Entity.create()

    local gunPos = Transform.getPosition(self.entity)
    local gunPosX = gunPos.x
    local gunPosY = gunPos.y
    Transform.setPosition(projEntity, gunPosX, gunPosY + 6)
    Component.addSprite(projEntity, SUPER_SHOOT_SPRITE, 0, 0, 4.0, 4.0)
    Component.addBoxCollider(projEntity, 32, 32)

    Component.setColliderLayer(projEntity, "PlayerBullet")
    Component.setColliderMask(projEntity, Layers.bit("Enemy"))

    Component.setOpacity(projEntity, 1.0)

    Component.connect(projEntity, "onCollisionEnter", self.entity, "onProjectileHit")

    -- In multiplayer this assigns a network id and makes ReplicationSystem
    -- announce the spawn + sync its position/sprite to every client.
    if isNetworked() then
        Net.replicate(projEntity, "Server", true, false)
    end

    table.insert(self.projectiles, {
        entity = projEntity,
        timer = PROJECTILE_LIFETIME,
        hitsRemaining = 3,
    })
end

function PlayerGun:onProjectileHit(bullet, other)
    local projectile
    for i = #self.projectiles, 1, -1 do
        if self.projectiles[i].entity == bullet then
            projectile = self.projectiles[i]
            break
        end
    end

    if projectile == nil then
        return
    end

    if projectile.hitsRemaining ~= nil then
        if Component.getColliderLayerName(other) ~= "Enemy" then
            return
        end
        projectile.hitsRemaining = projectile.hitsRemaining - 1
        if projectile.hitsRemaining > 0 then
            return
        end
    end

    Entity.queueDestroy(bullet)
    for i = #self.projectiles, 1, -1 do
        if self.projectiles[i].entity == bullet then
            table.remove(self.projectiles, i)
            break
        end
    end
end

function PlayerGun:onDestroy(entity)
    destroyProjectiles(self)
end

return PlayerGun
