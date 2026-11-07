local Enemy = {}

local SPRITES = {
    "assets/enemies/red-spinner/enemy1000.png",
    "assets/enemies/red-spinner/enemy1001.png",
    "assets/enemies/red-spinner/enemy1002.png",
    "assets/enemies/red-spinner/enemy1003.png",
    "assets/enemies/red-spinner/enemy1004.png",
    "assets/enemies/red-spinner/enemy1005.png",
    "assets/enemies/red-spinner/enemy1005.png",
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

local SPACING = 100.0

local PROJECTILE_SPRITE = "assets/enemies/red-spinner/laserbeam.png"
local PROJECTILE_INTERVAL = 1.0
local PROJECTILE_SPEED = 700.0
local PROJECTILE_LIFETIME = 3.0
local SCREEN_MARGIN = 128.0
local BODY_LAYER = "Enemy"
local BODY_MASK = Layers.bit("Player") + Layers.bit("PlayerBullet") + Layers.bit("Wall")

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

local function SortByX(ids)
    table.sort(ids, function(a, b)
        return Transform.getPosition(a).x < Transform.getPosition(b).x
    end)
    return ids
end

local function TrailPointAt(trail, dist)
    local walked = 0.0
    for i = 1, #trail - 1 do
        local a, b = trail[i], trail[i + 1]
        local dx, dy = b.x - a.x, b.y - a.y
        local seg = math.sqrt(dx * dx + dy * dy)
        if walked + seg >= dist then
            local t = (seg > 0) and ((dist - walked) / seg) or 0
            return { x = a.x + dx * t, y = a.y + dy * t }
        end
        walked = walked + seg
    end
    return nil
end

local function UpdateProjectiles(self, dt)
    for i = #self.projectiles, 1, -1 do
        local projectile = self.projectiles[i]
        Transform.translate(projectile.entity, -PROJECTILE_SPEED * dt, 0)

        projectile.timer = projectile.timer - dt
        local position = Transform.getPosition(projectile.entity)
        local outsideScreen = false
        if Camera ~= nil and Camera.viewport ~= nil then
            local viewport = Camera.viewport()
            outsideScreen = position.x < viewport.x - SCREEN_MARGIN or
                position.x > viewport.x + viewport.width + SCREEN_MARGIN or
                position.y < viewport.y - SCREEN_MARGIN or
                position.y > viewport.y + viewport.height + SCREEN_MARGIN
        end
        if projectile.timer <= 0.0 or outsideScreen then
            Entity.destroy(projectile.entity)
            table.remove(self.projectiles, i)
        end
    end
end

local function DestroyProjectiles(self)
    for _, projectile in ipairs(self.projectiles) do
        Entity.destroy(projectile.entity)
    end
    self.projectiles = {}
end

function Enemy:onCreate(entity)
    self.entity = entity
    self.frame = 1
    self.elapsed = 0.0
    self.exploding = false
    self.projectileTimer = PROJECTILE_INTERVAL
    self.projectiles = {}

    -- Without this the collision callback is never invoked, so the enemy can
    -- never be destroyed. bink.lua does the same in its onCreate.
    Component.connect(entity, "onCollisionEnter", entity, "onCollisionEnter")

    Component.setSpritePath(entity, SPRITES[self.frame])
    Component.addBoxCollider(entity, 32, 32)
    Component.setColliderLayer(entity, BODY_LAYER)
    Component.setColliderMask(entity, BODY_MASK)
    Component.addAudioSource(entity, "assets/effects/explosion.mp3", false, false, 1)

    if Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked() then
        Net.replicate(entity, "Server", true, true)
        Net.setSnapshotPriority(entity, 2)
    end

    self.linkIndex = nil
    self.trailKey = nil
    local parent = Component.getParent(entity)
    if parent ~= nil then
        self.trailKey = "trail_" .. tostring(parent)
        local sibs = Component.getChildren(parent)
        SortByX(sibs)
        for i, id in ipairs(sibs) do
            if id == entity then
                self.linkIndex = i
                break
            end
        end
    end
end

function Enemy:onUpdate(entity, dt)
    if not shouldSimulate(entity) then
        return
    end

    if not dt or dt <= 0 then
        dt = 1.0 / 60.0
    end

    -- Despawn enemy when it goes past the right edge of the screen
    local position = Transform.getPosition(entity)
    if Camera ~= nil and Camera.viewport ~= nil then
        local viewport = Camera.viewport()
        if position.x > viewport.x + viewport.width + SCREEN_MARGIN then
            Entity.queueDestroy(entity)
            return
        end
    end

    UpdateProjectiles(self, dt)

    if self.linkIndex ~= nil and self.trailKey ~= nil then
        local bus = _PataTrails
        local trail = (bus ~= nil) and bus[self.trailKey] or nil
        if trail ~= nil then
            local point = TrailPointAt(trail, self.linkIndex * SPACING)
            if point ~= nil then
                Transform.setPosition(entity, point.x, point.y)
            end
        end
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

    self.projectileTimer = self.projectileTimer - dt
    while self.projectileTimer <= 0.0 do
        self:fireProjectile()
        self.projectileTimer = self.projectileTimer + PROJECTILE_INTERVAL
    end

    self.elapsed = self.elapsed + dt

    while self.elapsed >= FRAME_DURATION do
        self.elapsed = self.elapsed - FRAME_DURATION
        self.frame = (self.frame % #SPRITES) + 1
        Component.setSpritePath(entity, SPRITES[self.frame])
    end
end

function Enemy:onCollisionEnter(this, other)
    if not shouldSimulate(this) then
        return
    end

    if self.exploding then
        return
    end

    -- Only die from player bullets, not from walls/floor
    if Component.getColliderLayerName(other) ~= "PlayerBullet" then
        return
    end

    DestroyProjectiles(self)
    self.exploding = true
    self.frame = 1
    self.elapsed = 0.0
    Component.playAudio(this)

    Component.setSpritePath(this, EXPLOSION[self.frame])
end

function Enemy:fireProjectile()
    local projectileEntity = Entity.create()
    local enemyPos = Transform.getPosition(self.entity)

    Transform.setPosition(projectileEntity, enemyPos.x - 24, enemyPos.y)
    Component.addSprite(projectileEntity, PROJECTILE_SPRITE, 0, 0, 4.0, 4.0)
    Component.setSpriteFlip(projectileEntity, true)
    Component.addBoxCollider(projectileEntity, 32, 32)
    Component.setColliderLayer(projectileEntity, "EnemyBullet")
    Component.setColliderMask(projectileEntity, Layers.bit("Player") + Layers.bit("Wall"))
    Component.setOpacity(projectileEntity, 1.0)
    Component.connect(projectileEntity, "onCollisionEnter", self.entity, "onProjectileHit")

    if Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked() then
        Net.replicate(projectileEntity, "Server", true, true)
    end

    table.insert(self.projectiles, {
        entity = projectileEntity,
        timer = PROJECTILE_LIFETIME,
    })
end

function Enemy:onProjectileHit(projectile, other)
    Entity.queueDestroy(projectile)
    for i = #self.projectiles, 1, -1 do
        if self.projectiles[i].entity == projectile then
            table.remove(self.projectiles, i)
            break
        end
    end
end

function Enemy:onDestroy(entity)
    DestroyProjectiles(self)
end

return Enemy
