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

-- Single player: second opponent of each country (entity "FighterP2_B")
local SWAP_DELAY = 2.0
local SECOND_OPPONENT = {
    England = "CAMMY", USA = "KEN", China = "DHALSIM", Thailand = "BLANKA", Japan = "ZANGIEF"
}

-- Background music per country: "a" plays against the first opponent and
-- "b" (if any) against the second; without "b" the same track keeps playing.
local SND = "assets/sound/"
local MUSIC_VOLUME = 0.5
local MUSIC = {
    USA      = { a = SND .. "08 Joe (USA, Kickboxer).mp3", b = SND .. "09 Mike (USA, Boxer).mp3" },
    England  = { a = SND .. "11 Birdie (England, Mohawk).mp3" },
    China    = { a = SND .. "14 Lee (China, Great Wall).mp3" },
    Thailand = { a = SND .. "17 Adon (Thailand, Sunset).mp3" },
    Japan    = { a = SND .. "06 Geki (Japan, Ninja).mp3", b = SND .. "05 Retsu (Japan, Shaolin Temple).mp3" },
}
local GAME_OVER_MUSIC = SND .. "23 Continue.mp3"
local WIN_SOUND = SND .. "you-win-street-fighter.mp3"
local LOSE_SOUND = SND .. "you-lose.mp3"
local ROUND_SOUND = SND .. "round.mp3"
local FIGHT_SOUND = SND .. "fight.mp3"
local ROUND_NUMBER = { [1] = SND .. "1.mp3", [2] = SND .. "2.mp3" }
local ROUND_TIMELINE = {
    { t = 0.0 }, { t = 0.95 }, { t = 1.75 }, { t = 2.4 },
}

local MENU_SCENE = "game_menu.scene"
local WIN_EXIT_DELAY, LOSE_EXIT_DELAY = 5.0, 8.0

local LOSE_TO_CONTINUE = 1.8

local function playMusic(path, loop)
    if path and Audio and Audio.playMusic then
        Audio.stopAllMusic()
        Audio.playMusic(path, MUSIC_VOLUME, loop)
    end
end

local P2_PATH = "assets/misc/koYell_p2.png"
local YELLOW_W, YELLOW_H = 324, 14
local HALF_W = 147
local P2_START = YELLOW_W - HALF_W

function Script:onCreate(entity)
    self.entity = entity
    self.gameOver = false
    FightState = { over = false, winner = nil }
    FighterHitStun, FighterHitSeq, FighterBlocking = {}, {}, {}
    ActiveOpponent = "A"
    FighterEntities = {}
    local country = SelectedCountry and SelectedCountry.name or "USA"
    self.music = MUSIC[country] or MUSIC.USA
    playMusic(self.music.a, true)
    self.round = 1
    self:startRound()
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

    for id = 1, 2 do
        FighterEntities[id] = self.fighters[id].body
    end

    local online = Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked()
    local reserveBody = (not online) and Entity.find("FighterP2_B") or nil
    if reserveBody then
        local children = Component.getChildren and Component.getChildren(reserveBody)
        self.reserve = {
            body = reserveBody,
            hitbox = children and #children > 0 and children[1] or nil,
            label = SECOND_OPPONENT[SelectedCountry and SelectedCountry.name or ""],
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
    Component.setOpacity(f.bar, 1.0)

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
        if Audio and Audio.playSound then Audio.playSound("assets/sound/punch.mp3", 0.4, false) end
    else
        if Audio and Audio.playSound then Audio.playSound("assets/sound/classic_punch.mp3", 1.0, false) end
        -- the victim's own voice: AudioSource in its prefab
        if victim.body and Component.playAudio then Component.playAudio(victim.body) end
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
    if winner.id == 1 and self.reserve then
        self.swapTimer = SWAP_DELAY
    elseif Audio and Audio.playSound then
        Audio.stopAllMusic()
        if winner.id == 1 then
            Audio.playSound(WIN_SOUND, 1.0, false)        -- last opponent beaten
            self.exitTimer = WIN_EXIT_DELAY
        else
            Audio.playSound(LOSE_SOUND, 1.0, false)       -- the player lost
            self.continueTimer = LOSE_TO_CONTINUE
            self.exitTimer = LOSE_EXIT_DELAY
        end
    end
end

function Script:nextOpponent()
    local r = self.reserve
    self.reserve = nil
    self.swapTimer = nil
    ActiveOpponent = "B"
    if self.music.b then playMusic(self.music.b, true) end
    self.round = self.round + 1
    self:startRound()
    local f = self.fighters[2]
    f.body, f.hitbox, f.health, f.hasHit = r.body, r.hitbox, MAX_HEALTH, false
    FighterEntities[2] = r.body
    FighterHitStun[2] = 0.0
    FighterBlocking[2] = false
    self:refreshBar(f)
    local label = Entity.find("P2_Text")
    if label and r.label then
        Component.setText(label, r.label)
    end
    self.gameOver = false
    FightState = { over = false, winner = nil }
    log.info("[FighterHealth] next opponent")
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

function Script:startRound()
    RoundIntro = true
    self.introTime, self.introStep = 0.0, 1
end

function Script:updateRoundIntro(dt)
    if not self.introTime then return end
    self.introTime = self.introTime + dt
    local nextEvent = ROUND_TIMELINE[self.introStep]
    while nextEvent and self.introTime >= nextEvent.t do
        local step = self.introStep
        if Audio and Audio.playSound then
            if step == 1 then Audio.playSound(ROUND_SOUND, 1.0, false)
            elseif step == 2 and ROUND_NUMBER[self.round] then
                Audio.playSound(ROUND_NUMBER[self.round], 1.0, false)
            elseif step == 3 then Audio.playSound(FIGHT_SOUND, 1.0, false) end
        end
        if step == 4 then
            RoundIntro = false
            self.introTime = nil
            return
        end
        self.introStep = step + 1
        nextEvent = ROUND_TIMELINE[self.introStep]
    end
end

function Script:onUpdate(entity, dt)
    self:updateRoundIntro(dt)
    if self.exitTimer then
        self.exitTimer = self.exitTimer - dt
        if self.exitTimer <= 0.0 then
            self.exitTimer = nil
            if Audio and Audio.stopAllMusic then Audio.stopAllMusic() end
            Scene.load(MENU_SCENE)
            return
        end
    end
    if self.continueTimer then
        self.continueTimer = self.continueTimer - dt
        if self.continueTimer <= 0.0 then
            self.continueTimer = nil
            playMusic(GAME_OVER_MUSIC, false)
        end
    end
    if self.gameOver then
        if self.swapTimer then
            self.swapTimer = self.swapTimer - dt
            if self.swapTimer <= 0.0 then
                self:nextOpponent()
            end
        end
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
