local Bullet = {}

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

function Bullet:onCreate(sprites, speed, lifetime, hitsRemaining, direction, startX, startY)
    self.sprites = sprites
    self.speed = speed
    self.lifetime = lifetime
    self.hitsRemaining = hitsRemaining
    self.direction = direction
    self.startX = startX
    self.startY = startY
    self.entity = entity

    -- On clients, spawn enemy bullets locally from the server's spawn event.
    if Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked() and not Net.isServer() then
        Net.on("spawn_bullet", function(peer, data)
            local x = data.x
            local y = data.y
            local e = Entity.create()
            Transform.setPosition(e, x, y)
            Component.addSprite(e, PROJECTILE_SPRITE, 0, 0, 4.0, 4.0)
            Component.setSpriteFlip(e, true)
            Component.addBoxCollider(e, 32, 32)
            Component.setColliderLayer(e, "EnemyBullet")
            Component.setColliderMask(e, Layers.bit("Player") + Layers.bit("Wall"))
            Component.setOpacity(e, 1.0)
            Component.addScript(e, "scripts/bulletProjectile.lua", true)
        end)
    end
end

function Bullet:onUpdate(entity, dt)
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

    if not shouldSimulate(entity) then
        return
    end

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

function Bullet:onCollisionEnter(this, other)
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

function Bullet:fireProjectile()
    local enemyPos = Transform.getPosition(self.entity)
    local x = enemyPos.x - 24
    local y = enemyPos.y

    local projectileEntity = Entity.create()
    Transform.setPosition(projectileEntity, x, y)
    Component.addSprite(projectileEntity, PROJECTILE_SPRITE, 0, 0, 4.0, 4.0)
    Component.setSpriteFlip(projectileEntity, true)
    Component.addBoxCollider(projectileEntity, 32, 32)
    Component.setColliderLayer(projectileEntity, "EnemyBullet")
    Component.setColliderMask(projectileEntity, Layers.bit("Player") + Layers.bit("Wall"))
    Component.setOpacity(projectileEntity, 1.0)
    Component.addScript(projectileEntity, "scripts/bulletProjectile.lua", true)

    if Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked() and Net.isServer() then
        Net.broadcast("spawn_bullet", { x = x, y = y }, true, 1)
    end
end

function Bullet:onDestroy(entity)
end

return Bullet
