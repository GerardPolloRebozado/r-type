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

    -- P1 half = this entity
    self.fighters[1].bar = entity

    -- P2 half = new sprite on top of the same place
    local p2 = Entity.create()
    Component.addSprite(p2, P2_PATH, 0, 0, self.sx, self.sy)
    Transform.setPosition(p2, self.baseX + P2_START * self.sx, self.baseY, self.baseZ + 1)
    self.fighters[2].bar = p2

    self:refreshBar(self.fighters[1])
    self:refreshBar(self.fighters[2])
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
    if w < 1 then
        Component.setSpriteRect(f.bar, 0, 0, 0, 0)
        return
    end

    if f.id == 1 then
        -- P1: anchored to the KO logo, the visible part shrinks towards the left edge
        local srcX = HALF_W - w
        Component.setSpriteRect(f.bar, srcX, 0, w, YELLOW_H)
        Transform.setPosition(f.bar, self.baseX + srcX * self.sx, self.baseY, self.baseZ + 1)
    else
        -- P2: anchored to the KO logo, the visible part shrinks towards the right edge
        Component.setSpriteRect(f.bar, P2_START, 0, w, YELLOW_H)
        Transform.setPosition(f.bar, self.baseX + P2_START * self.sx, self.baseY, self.baseZ + 1)
    end
end

function Script:applyDamage(attacker, victim, amount)
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
    self:checkHit(p1, p2)
    if not self.gameOver then
        self:checkHit(p2, p1)
    end
end

function Script:onDestroy(entity)
    log.info("[FighterHealth] destroyed")
end

return Script
