-- Simple opponent "AI": drives player 2 with predefined movement patterns.
--
-- It only writes inputs (AIInputs[2]); FighterMovement.lua reads them in
-- place of the keyboard, so the AI uses exactly the same moves as a player.
--
-- Difficulty: every pause (the "w" of a step and the think time between
-- patterns) is multiplied by a delay multiplier.
--   1.0 = normal, > 1 = slower / easier, 0 = no pauses (hardest).
-- A future difficulty menu only has to set the global before the fight:
--     AIDelayMultiplier = 2.0
-- Set GameMode = "versus" to disable the AI (two human players).

local Script = {}

local DEFAULT_DELAY_MULTIPLIER = 1.0
local THINK_TIME = 0.35   -- pause between patterns, before the multiplier
local MIN_GAP = 0.06      -- inputs are released at least this long between steps
                          -- (a second Jump press needs a release in between)
local FAR_DISTANCE, NEAR_DISTANCE = 320.0, 150.0

-- anti-spam: minimum time between two attack steps (not scaled by the multiplier)
local ATTACK_DELAY = 0.3
local ATTACK_ACTIONS = {
    LightPunch = true, MediumPunch = true, HeavyPunch = true,
    LightKick = true, MediumKick = true, HeavyKick = true,
}

-- reactions: when the player attacks (or throws a fireball) nearby
local REACTION_TIME = 0.15     -- before the multiplier
local REACT_RANGE = 230.0
local REACT_COOLDOWN = 0.9     -- so it does not react to every single hit
local BLOCK_CHANCE = 0.55      -- otherwise it tries to hit back
local FIREBALL_RANGE = 380.0

-- Each step: a = actions held, t = how long (s), w = pause afterwards (s, scaled).
-- "Forward" / "Back" are relative to the opponent; the rest are FighterMovement actions.
local PATTERNS = {
    approach = {
        { a = { "Forward" }, t = 0.7 },
    },
    jump_in = {
        { a = { "Forward", "Jump" }, t = 0.1, w = 0.1 },
        { a = { "Forward" }, t = 0.3 },
        { a = { "LightKick" }, t = 0.2, w = 0.3 },
    },
    poke = {
        { a = { "Forward" }, t = 0.25 },
        { a = { "LightPunch" }, t = 0.12, w = 0.2 },
        { a = { "MediumKick" }, t = 0.12, w = 0.3 },
    },
    double_jump_dodge = {
        { a = { "Jump" }, t = 0.08, w = 0.25 },
        { a = { "Jump", "Back" }, t = 0.08, w = 0.35 },
    },
    sweep = {
        { a = { "Crouch", "LightKick" }, t = 0.25, w = 0.3 },
    },
    combo = {
        { a = { "LightPunch" }, t = 0.1, w = 0.15 },
        { a = { "MediumPunch" }, t = 0.1, w = 0.15 },
        { a = { "HeavyKick" }, t = 0.12, w = 0.4 },
    },
    low_punch = {
        { a = { "Crouch", "LightPunch" }, t = 0.2, w = 0.2 },
    },
    block = {
        { a = { "Block" }, t = 0.7, w = 0.1 },
        { a = { "Back" }, t = 0.3 },
    },
    retreat = {
        { a = { "Back" }, t = 0.5 },
    },
}

-- reactions (when the player attacks)
PATTERNS.parry = {              -- block, then riposte
    { a = { "Block" }, t = 0.45, w = 0.05 },
    { a = { "LightPunch" }, t = 0.1, w = 0.3 },
}
PATTERNS.counter = {            -- hit back
    { a = { "LightPunch" }, t = 0.1, w = 0.15 },
    { a = { "MediumKick" }, t = 0.12, w = 0.3 },
}
PATTERNS.fireball_jump = {      -- jump over a fireball
    { a = { "Jump" }, t = 0.08, w = 0.2 },
    { a = { "Jump", "Forward" }, t = 0.08, w = 0.3 },
}

-- which patterns can be picked at each distance
local POOLS = {
    far  = { "approach", "approach", "jump_in" },
    mid  = { "poke", "jump_in", "double_jump_dodge", "sweep", "approach" },
    near = { "combo", "low_punch", "block", "retreat", "sweep" },
}

local function delayMultiplier()
    local m = AIDelayMultiplier
    if type(m) ~= "number" then m = DEFAULT_DELAY_MULTIPLIER end
    return math.max(m, 0.0)
end

local function isOnline()
    return Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked()
end

function Script:onCreate(entity)
    self.entity = entity
    self.pattern, self.step = nil, 0
    self.timer, self.phase = 0.0, "think"   -- think | hold | wait
    self.waitTime = 0.0

    AIInputs = AIInputs or {}
    AIEnabled = AIEnabled or {}
    local enabled = (GameMode ~= "versus") and not isOnline()
    AIEnabled[2] = enabled
    AIInputs[2] = {}
    self.attackCd, self.reactCd = 0.0, 0.0
    if math.randomseed and os and os.time then
        math.randomseed(os.time())
    end
end

function Script:fighter(id)
    return (FighterEntities and FighterEntities[id]) or Entity.find("FighterP" .. id)
end

function Script:distance()
    local me, foe = self:fighter(2), self:fighter(1)
    if not me or not foe then return nil end
    local a, b = Transform.getPosition(me), Transform.getPosition(foe)
    if not a or not b then return nil end
    return math.abs(b.x - a.x), (b.x >= a.x) and 1 or -1
end

function Script:hasAttack(step)
    for _, a in ipairs(step.a) do
        if ATTACK_ACTIONS[a] then return true end
    end
    return false
end

-- starts a step; attack steps wait until the anti-spam delay is over
function Script:beginStep()
    local step = self.pattern[self.step]
    if self:hasAttack(step) and self.attackCd > 0.0 then
        self.phase, self.timer = "gap", self.attackCd
    else
        self.phase, self.timer = "hold", step.t
    end
end

function Script:playerAttacking()
    local foe = self:fighter(1)
    if not foe then return false end
    if self.foeHitbox == nil and Component.getChildren then
        local children = Component.getChildren(foe)
        self.foeHitbox = (children and #children > 0) and children[1] or false
    end
    local attacking = self.foeHitbox and Component.getColliderEnabled(self.foeHitbox) or false
    return attacking
end

function Script:incomingFireball()
    local me = self:fighter(2)
    local mp = me and Transform.getPosition(me)
    if not mp or not Fireballs then return false end
    for _, fb in ipairs(Fireballs) do
        if fb.owner == 1 and not fb.impact and math.abs(fb.x - mp.x) < FIREBALL_RANGE then
            return true
        end
    end
    return false
end

function Script:react()
    if self.reactCd > 0.0 then return nil end
    if self:incomingFireball() then
        return "fireball_jump"
    end
    local dist = self:distance()
    if dist and dist < REACT_RANGE and self:playerAttacking() then
        return (math.random() < BLOCK_CHANCE) and "parry" or "counter"
    end
    return nil
end

function Script:startPattern(pattern)
    self.pattern = pattern
    self.step = 1
    self:beginStep()
end

function Script:choosePattern()
    local dist = self:distance()
    local range = "mid"
    if dist then
        if dist > FAR_DISTANCE then range = "far"
        elseif dist < NEAR_DISTANCE then range = "near" end
    end
    local pool = POOLS[range]
    self:startPattern(PATTERNS[pool[math.random(#pool)]])
end

function Script:setInputs(actions, towards)
    local inputs = {}
    if actions then
        for _, a in ipairs(actions) do
            if a == "Forward" then
                inputs[towards > 0 and "Right" or "Left"] = true
            elseif a == "Back" then
                inputs[towards > 0 and "Left" or "Right"] = true
            else
                inputs[a] = true
            end
        end
    end
    AIInputs[2] = inputs
end

function Script:onUpdate(entity, dt)
    if not (AIEnabled and AIEnabled[2]) then
        return
    end
    if FightState and FightState.over then
        AIInputs[2] = {}
        self.pattern, self.phase, self.timer = nil, "think", THINK_TIME
        return
    end

    local _, towards = self:distance()
    towards = towards or 1

    self.timer = self.timer - dt
    self.attackCd = math.max(self.attackCd - dt, 0.0)
    self.reactCd = math.max(self.reactCd - dt, 0.0)

    -- reactions interrupt whatever it was doing (after a reaction delay)
    if self.phase ~= "react" then
        local reaction = self:react()
        if reaction then
            self.reaction = reaction
            self.phase = "react"
            self.timer = REACTION_TIME * delayMultiplier()
            self.reactCd = REACT_COOLDOWN
            AIInputs[2] = {}
        end
    end

    if self.phase == "react" then
        AIInputs[2] = {}
        if self.timer <= 0.0 then
            self:startPattern(PATTERNS[self.reaction])
        end
    elseif self.phase == "gap" then
        AIInputs[2] = {}
        if self.timer <= 0.0 then
            self.phase, self.timer = "hold", self.pattern[self.step].t
        end
    elseif self.phase == "think" then
        AIInputs[2] = {}
        if self.timer <= 0.0 then
            self:choosePattern()
        end
    elseif self.phase == "hold" then
        self:setInputs(self.pattern[self.step].a, towards)
        if self.timer <= 0.0 then
            local s = self.pattern[self.step]
            if self:hasAttack(s) then self.attackCd = ATTACK_DELAY end
            self.phase = "wait"
            self.timer = math.max((s.w or 0.0) * delayMultiplier(), MIN_GAP)
        end
    else -- wait: release everything
        AIInputs[2] = {}
        if self.timer <= 0.0 then
            self.step = self.step + 1
            if self.pattern[self.step] then
                self:beginStep()
            else
                self.phase = "think"
                self.timer = THINK_TIME * delayMultiplier()
            end
        end
    end
end

function Script:onDestroy(entity)
    if AIEnabled then AIEnabled[2] = false end
    if AIInputs then AIInputs[2] = {} end
end

return Script
