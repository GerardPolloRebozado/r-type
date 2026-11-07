local Script = {}

local MAX_HEALTH = 100

local DAMAGE = {
    LightPunch  = 5,
    MediumPunch = 8,
    HeavyPunch  = 12,
    LightKick   = 6,
    MediumKick  = 10,
    HeavyKick   = 14
}

local HIT_STUN = 0.3
local BLOCK_CHIP = 0.2

local ATTACK_PRIORITY = {
    "HeavyKick", "HeavyPunch", "MediumKick", "MediumPunch", "LightKick", "LightPunch"
}

local P2_PATH = "assets/misc/koYell_p2.png"
local YELLOW_W, YELLOW_H = 324, 14
local HALF_W = 147
local P2_START = YELLOW_W - HALF_W

function Script:onCreate(entity)
    self.entity = entity
    self.gameOver = false
    FightState = { over = false, winner = nil }
    FighterHitStun, FighterHitSeq, FighterBlocking = {}, {}, {}
    -- called by FighterMovement when a fireball touches the opponent
    FireballHit = function(attackerId, amount)
        if self.gameOver then return end
        local victim = self.fighters[attackerId == 1 and 2 or 1]
        local attacker = self.fighters[attackerId]
        if victim and attacker then
            self:applyDamage(attacker, victim, amount)
        end
    end

    self.fighters = {}
    for id = 1, 2 do
        local body = Entity.find("FighterP" .. id)
        local hitbox = nil
        if body and Component.getChildren then
            local children = Component.getChildren(body)
            if children and #children > 0 then
                hitbox = children[1]
            end
        end
        if not body then
            log.error("[FighterHealth] Entity 'FighterP" .. id .. "' not found")
        end
        self.fighters[id] = {
            id = id,
            body = body,
            hitbox = hitbox,
            health = MAX_HEALTH,
            hasHit = false,
            bar = nil
        }
    end

    local pos = Transform.getPosition(entity)
    local scale = Component.getSpriteScale(entity)
    self.baseX, self.baseY, self.baseZ = pos.x, pos.y, pos.z
    self.sx = scale and scale.x or 1.0
    self.sy = scale and scale.y or 1.0

    -- half = this entity
    self.fighters[1].bar = entity

    -- half = new sprite on top of the same place
    local p2 = Entity.create()
    Component.addSprite(p2, P2_PATH, 0, 0, self.sx, self.sy)
    Transform.setPosition(p2, self.baseX + P2_START * self.sx, self.baseY, self.baseZ + 1)
    self.fighters[2].bar = p2
    self.barsReady = false
end

function Script:currentAttack(playerId)
    if not Controller or not Controller.isActionActive then
        return nil
    end
    for _, action in ipairs(ATTACK_PRIORITY) do
        if Controller.isActionActive(action, playerId) then
            return action
        end
    end
    return nil
end

function Script:refreshBar(f)
    local w = math.floor(HALF_W * math.max(f.health, 0) / MAX_HEALTH + 0.5)
    local ok
    if w < 1 then
        Component.setOpacity(f.bar, 0.0)
        return true
    end

    if f.id == 1 then
        -- anchored to the KO logo, left edge
        local srcX = HALF_W - w
        ok = Component.setSpriteRect(f.bar, srcX, 0, w, YELLOW_H)
        Transform.setPosition(f.bar, self.baseX + srcX * self.sx, self.baseY, self.baseZ + 1)
    else
        -- anchored to the KO logo, right edge
        ok = Component.setSpriteRect(f.bar, P2_START, 0, w, YELLOW_H)
        Transform.setPosition(f.bar, self.baseX + P2_START * self.sx, self.baseY, self.baseZ + 1)
    end
    return ok
end

function Script:applyDamage(attacker, victim, amount)
    local blocked = FighterBlocking and FighterBlocking[victim.id]
    if blocked then
        amount = math.floor(amount * BLOCK_CHIP)
    else
        FighterHitStun = FighterHitStun or {}
        FighterHitSeq = FighterHitSeq or {}
        FighterHitStun[victim.id] = HIT_STUN
        FighterHitSeq[victim.id] = (FighterHitSeq[victim.id] or 0) + 1
    end
    victim.health = math.max(victim.health - amount, 0)
    log.info(string.format("[FighterHealth] P%d hits P%d for %d (P%d: %d/%d)",
        attacker.id, victim.id, amount, victim.id, victim.health, MAX_HEALTH))
    self:refreshBar(victim)

    if victim.health <= 0 then
        self:onKO(attacker, victim)
    end
end

function Script:onKO(winner, loser)
    self.gameOver = true
    log.info(string.format("[FighterHealth] KO! P%d wins", winner.id))
    FightState = { over = true, winner = winner.id }
end

function Script:checkHit(attacker, victim)
    if not attacker.hitbox or not victim.body then
        return
    end

    local active = Component.getColliderEnabled(attacker.hitbox)
    if not active then
        attacker.hasHit = false
        return
    end
    if attacker.hasHit then
        return
    end

    local touching = Component.getCollisions(attacker.hitbox)
    for _, other in ipairs(touching) do
        if other == victim.body then
            local action = self:currentAttack(attacker.id)
            local amount = DAMAGE[action] or DAMAGE.LightPunch
            attacker.hasHit = true
            self:applyDamage(attacker, victim, amount)
            return
        end
    end
end

function Script:onUpdate(entity, dt)
    if self.gameOver then
        return
    end
    local p1, p2 = self.fighters[1], self.fighters[2]
    if not self.barsReady then
        local ok1 = self:refreshBar(p1)
        local ok2 = self:refreshBar(p2)
        self.barsReady = ok1 and ok2
    end
    self:checkHit(p1, p2)
    if not self.gameOver then
        self:checkHit(p2, p1)
    end
end

function Script:onDestroy(entity)
    log.info("[FighterHealth] destroyed")
end

return Script
