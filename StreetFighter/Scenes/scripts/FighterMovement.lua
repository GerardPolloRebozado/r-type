local Script = {}

local PLAYER_KEYS = {
    [1] = {
        Left = 65,
        Right = 68,
        Jump = 87,
        Crouch = 83,
        LightPunch = 85,
        MediumPunch = 73,
        HeavyPunch = 79,
        LightKick = 74,
        MediumKick = 75,
        HeavyKick = 76,
        Block = 340, -- Left Shift
        Hadouken = 72 -- H
    },
    [2] = {
        Left = 324,
        Right = 326,
        Jump = 328,
        Crouch = 325,
        LightPunch = 321,
        MediumPunch = 322,
        HeavyPunch = 323,
        LightKick = 327,
        MediumKick = 329,
        HeavyKick = 320,
        Block = 330, -- Numpad .
        Hadouken = 335 -- Numpad Enter
    }
}

local function detectPlayerId(entity)
    local name = ""
    if Entity and Entity.getName then
        name = Entity.getName(entity) or ""
    end
    local id = tonumber(name:match("[Pp](%d+)"))
    if not id then
        id = tonumber(name:match("(%d+)$"))
    end
    return id or 1
end

local function isNetworked()
    return Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked()
end

local function isServer()
    return Net ~= nil and Net.isServer ~= nil and Net.isServer()
end

local function shouldSimulate()
    if not isNetworked() then return true end
    return isServer()
end


NetInputs = NetInputs or {}
Fireballs = Fireballs or {}

local FB_SHEET = "assets/fighters/sprites_hadouken.png"
local FB_FLY = { {10, 59}, {66, 59}, {120, 59}, {175, 59}, {230, 59}, {284, 59} }
local FB_W, FB_H = 46, 25
local FB_HIT = { 66, 6, 48, 46 }       -- impact burst
local FB_SPEED, FB_DAMAGE, FB_MAX_X = 420.0, 10, 1500.0
local FB_SCALE = 1.66
local CAST_TIME, CAST_FIRE_AT, CAST_COOLDOWN = 0.3, 0.15, 0.6

-- anti-spam: an attack needs a fresh key press, lasts ATTACK_ACTIVE seconds and
-- the next one can only start ATTACK_DELAY seconds after it ends
local ATTACK_ACTIVE, ATTACK_DELAY = 0.3, 0.25
local P1_START_X = 346.45
local PARK_X, PARK_Y = -2000.0, -2000.0

local SYNC_FIELDS = { "netAnim", "netFlip" }

function Script:onCreate(entity)
    self.entity = entity
    self.playerId = detectPlayerId(entity)
    self.netAnim = ""
    self.netFlip = false
    self.netSynced = false

    self.moveSpeed = 220.0
    self.jumpForce = -420.0
    self.doubleJumpForce = -380.0 -- slightly weaker than the first jump
    self.maxJumps = 2
    self.jumpsUsed = 0
    self.jumpHeld = false
    self.gravity = 980.0

    self.vy = 0.0
    self.isGrounded = true
    self.isCrouching = false
    self.groundY = nil
    self.currentAnim = ""
    self.facingRight = (self.playerId == 1)
    self.hitboxEntity = nil

    -- "FighterP2_B" is the reserve opponent: parked until FighterHealth activates it
    local name = Entity.getName and Entity.getName(entity) or ""
    self.isReserve = name:match("_B$") ~= nil
    local home = Transform.getPosition(entity)
    if home then
        self.homeX, self.homeY, self.homeZ = home.x, home.y, home.z
    end

    self:setupKeybinds()
end

-- Direction each player's sprite sheet is drawn facing (Ryu faces right,
-- the SF2 Sagat sheet faces left). Flip only when the current direction differs.
local SPRITE_FACES_RIGHT = { [1] = true, [2] = false }

function Script:applyFlip()
    local native = SPRITE_FACES_RIGHT[self.playerId]
    if native == nil then native = true end
    local flip = (self.facingRight ~= native)
    self.netFlip = flip
    if Animation and Animation.setFlip then
        Animation.setFlip(self.entity, flip)
    elseif Component and Component.setSpriteFlip then
        Component.setSpriteFlip(self.entity, flip)
    end
end

function Script:setupKeybinds()
    if not Controller or not Controller.addKeybind then
        return
    end
    local bindId = isNetworked() and 1 or self.playerId
    local keys = PLAYER_KEYS[bindId] or PLAYER_KEYS[1]
    for action, key in pairs(keys) do
        Controller.addKeybind(action, key, Controller.KEYBOARD, 0, bindId, isNetworked())
    end
end

function Script:isAction(actionName)
    if isNetworked() then
        local input
        if self.slot ~= nil then
            input = Net.input and Net.input(self.entity)
        else
            input = NetInputs[self.playerId]
        end
        return input ~= nil and input[actionName] == true
    end
    -- single player: FighterAI.lua drives player 2 instead of the keyboard
    if self.playerId == 2 and AIEnabled and AIEnabled[2] then
        local inputs = AIInputs and AIInputs[2]
        return inputs ~= nil and inputs[actionName] == true
    end
    if Controller and Controller.isActionActive then
        return Controller.isActionActive(actionName, self.playerId)
    end
    return false
end

-- Plays clipName; if the fighter's prefab has no such clip, plays fallback.
function Script:playAnim(clipName, fallback, restartIfSame)
    if not Animation or not Animation.play then
        return
    end
    if self.requestedAnim == clipName and not restartIfSame then
        return
    end
    local played = clipName
    local ok = Animation.play(self.entity, clipName, restartIfSame or false)
    if not ok and fallback then
        played = fallback
        Animation.play(self.entity, fallback, restartIfSame or false)
    end
    self.requestedAnim = clipName
    self.currentAnim = played
    self.netAnim = played
end

function Script:spawnFireball(pos)
    local body = Component.getColliderSize(self.entity)
    local bodyW = body and body.w or 60.0
    local e = Entity.create()
    if not Component.addSprite(e, FB_SHEET, 0, 0, FB_SCALE, FB_SCALE) then
        Entity.destroy(e)
        return
    end
    Component.setSpriteRect(e, FB_FLY[1][1], FB_FLY[1][2], FB_W, FB_H)
    -- the sheet draws the fireball flying left
    Component.setSpriteFlip(e, self.facingRight)
    local w = FB_W * FB_SCALE
    local x = self.facingRight and (pos.x + bodyW) or (pos.x - w)
    local y = pos.y + 30.0
    Transform.setPosition(e, x, y, pos.z)
    table.insert(Fireballs, { owner = self.playerId, entity = e, x = x, y = y, z = pos.z,
        dir = self.facingRight and 1 or -1, frame = 1, timer = 0.0, impact = nil })
end

function Script:clearFireballs()
    for i = #Fireballs, 1, -1 do
        local fb = Fireballs[i]
        if fb.owner == self.playerId then
            Entity.destroy(fb.entity)
            table.remove(Fireballs, i)
        end
    end
end

function Script:updateFireballs(dt)
    for i = #Fireballs, 1, -1 do
        local fb = Fireballs[i]
        if fb.owner == self.playerId then
            local remove = false
            if fb.impact then
                fb.impact = fb.impact - dt
                remove = fb.impact <= 0
            else
                fb.x = fb.x + fb.dir * FB_SPEED * dt
                Transform.setPosition(fb.entity, fb.x, fb.y, fb.z)
                fb.timer = fb.timer + dt
                if fb.timer >= 0.06 then
                    fb.timer = 0.0
                    fb.frame = fb.frame % #FB_FLY + 1
                    Component.setSpriteRect(fb.entity, FB_FLY[fb.frame][1], FB_FLY[fb.frame][2], FB_W, FB_H)
                end
                local other = fb.owner == 1 and 2 or 1
                local victim = (FighterEntities and FighterEntities[other]) or Entity.find("FighterP" .. other)
                if victim then
                    local vp = Transform.getPosition(victim)
                    local vs = Component.getColliderSize(victim)
                    local fw, fh = FB_W * FB_SCALE, FB_H * FB_SCALE
                    if vp and vs and fb.x < vp.x + vs.w and fb.x + fw > vp.x
                        and fb.y < vp.y + vs.h and fb.y + fh > vp.y then
                        if FireballHit then FireballHit(fb.owner, FB_DAMAGE) end
                        fb.impact = 0.12
                        Component.setSpriteRect(fb.entity, FB_HIT[1], FB_HIT[2], FB_HIT[3], FB_HIT[4])
                    end
                end
                if math.abs(fb.x) > FB_MAX_X then remove = true end
            end
            if remove then
                Entity.destroy(fb.entity)
                table.remove(Fireballs, i)
            end
        end
    end
end

function Script:findHitboxEntity()
    if self.hitboxEntity ~= nil then
        return self.hitboxEntity
    end

    if Component and Component.getChildren then
        local children = Component.getChildren(self.entity)
        if children and #children > 0 then
            self.hitboxEntity = children[1]
            return self.hitboxEntity
        end
    end
    return nil
end

function Script:setHitboxActive(active, offsetX, offsetY, width, height)
    local hb = self:findHitboxEntity()
    if not hb or not Component or not Component.setColliderEnabled then
        return
    end

    Component.setColliderEnabled(hb, active)
    if not active then
        return
    end

    local w = width or 60.0
    local h = height or 40.0
    Component.setColliderSize(hb, w, h)

    local pos = Transform.getPosition(self.entity)
    local body = Component.getColliderSize(self.entity)
    local bodyW = body and body.w or 60.0
    local bodyH = body and body.h or 120.0
    local centerX = pos.x + bodyW / 2.0

    local x
    if self.facingRight then
        x = centerX
    else
        x = centerX - w
    end
    local y = pos.y + bodyH / 2.0 - h / 2.0 + (offsetY or 0.0)
    Transform.setPosition(hb, x, y, pos.z)
end

function Script:onUpdate(entity, dt)
    if dt <= 0 then
        return
    end

    if isNetworked() and not self.netSynced and Net.sync then
        self.netSynced = Net.sync(entity, self, SYNC_FIELDS) == true
    end

    if not shouldSimulate() then
        if self.netAnim ~= nil and self.netAnim ~= "" and self.netAnim ~= self.currentAnim then
            Animation.play(entity, self.netAnim, false)
            self.currentAnim = self.netAnim
        end
        if self.netFlip ~= self.appliedFlip then
            if Animation and Animation.setFlip then
                Animation.setFlip(entity, self.netFlip == true)
            elseif Component and Component.setSpriteFlip then
                Component.setSpriteFlip(entity, self.netFlip == true)
            end
            self.appliedFlip = self.netFlip
        end
        return
    end

    -- single player: only the active opponent (A, then B) is on stage
    if self.playerId == 2 and (self.isReserve and "B" or "A") ~= (ActiveOpponent or "A") then
        Transform.setPosition(entity, PARK_X, PARK_Y)
        self:setHitboxActive(false)
        self.retired = true
        return
    elseif self.retired then
        self.retired = false
        if self.homeX then
            Transform.setPosition(entity, self.homeX, self.homeY, self.homeZ)
            self.groundY = self.homeY
        end
        self.vy, self.isGrounded = 0.0, true
        self.requestedAnim = nil
    end

    self.slot = nil
    if isServer() and Net.playerSlot then
        self.slot = Net.playerSlot(entity)
    end
    if isNetworked() and not self.roleLogged then
        self.roleLogged = true
        log.info(string.format("[FighterMovement][net] entity=%s name=%s server=%s slot=%s playerId=%s",
            tostring(entity), tostring(Entity.getName(entity)), tostring(isServer()),
            tostring(self.slot), tostring(self.playerId)))
    end
    if self.slot ~= nil then
        if self.slot >= 1 then
            if self.slot == 1 then
                NetInputs[2] = (Net.input and Net.input(entity)) or {}
                NetP2Alive = 3
            end
            Transform.setPosition(entity, PARK_X, PARK_Y)
            self:setHitboxActive(false)
            return
        end
        self.playerId = 1
        if not self.positioned then
            local cur = Transform.getPosition(entity)
            Transform.setPosition(entity, P1_START_X, FighterStartY or cur.y, cur.z)
            self.positioned = true
        end
    end

    if self.slot == nil and isNetworked() and isServer() and self.playerId == 2 then
        local here = Transform.getPosition(entity)
        if here and not self.homeX then
            self.homeX, self.homeY, self.homeZ = here.x, here.y, here.z
            self.groundY = here.y
            FighterStartY = here.y
        end
        local connected = (NetP2Alive or 0) > 0
        NetP2Alive = connected and (NetP2Alive - 1) or 0
        if not connected then
            Transform.setPosition(entity, PARK_X, PARK_Y)
            self:setHitboxActive(false)
            self.parked = true
            return
        elseif self.parked then
            -- client 2 just joined: back to the starting spot
            Transform.setPosition(entity, self.homeX, self.homeY, self.homeZ)
            self.vy, self.isGrounded = 0.0, true
            self.parked = false
        end
    end

    --  the end pose
    if FightState and FightState.over then
        self:clearFireballs()
        self:setHitboxActive(false)
        if FightState.winner == self.playerId then
            self:playAnim("victory")
        else
            self:playAnim("knockdown")
        end
        return
    end

    local pos = Transform.getPosition(entity)
    if not pos then
        return
    end

    -- hit stun, set by FighterHealth when this fighter takes damage
    FighterHitStun = FighterHitStun or {}
    FighterHitSeq = FighterHitSeq or {}
    FighterBlocking = FighterBlocking or {}
    local stun = FighterHitStun[self.playerId] or 0.0
    local stunned = stun > 0.0
    if stunned then
        FighterHitStun[self.playerId] = math.max(stun - dt, 0.0)
    end

    if not self.groundY then
        self.groundY = pos.y
    end

    self:updateFireballs(dt)
    if self.castTimer and self.castTimer > 0.0 then
        self.castTimer = self.castTimer - dt
        if not self.castFired and self.castTimer <= CAST_TIME - CAST_FIRE_AT then
            self.castFired = true
            self:spawnFireball(pos)
        end
    end
    self.fireCooldown = math.max((self.fireCooldown or 0.0) - dt, 0.0)
    local casting = (self.castTimer or 0.0) > 0.0
    if stunned then
        self.castTimer = 0.0
        casting = false
    end

    local canAct = not stunned and not casting
    local leftActive = canAct and self:isAction("Left")
    local rightActive = canAct and self:isAction("Right")
    local jumpActive = canAct and self:isAction("Jump")
    local crouchActive = canAct and self:isAction("Crouch")
    local blockActive = canAct and self:isAction("Block")

    local lightPunch = canAct and self:isAction("LightPunch")
    local mediumPunch = canAct and self:isAction("MediumPunch")
    local heavyPunch = canAct and self:isAction("HeavyPunch")
    local anyPunch = lightPunch or mediumPunch or heavyPunch

    local lightKick = canAct and self:isAction("LightKick")
    local mediumKick = canAct and self:isAction("MediumKick")
    local heavyKick = canAct and self:isAction("HeavyKick")
    local anyKick = lightKick or mediumKick or heavyKick

    self.attackTimer = math.max((self.attackTimer or 0.0) - dt, 0.0)
    self.attackCd = math.max((self.attackCd or 0.0) - dt, 0.0)
    if stunned then
        self.attackTimer = 0.0 -- being hit cancels the attack
    end
    local attackHeld = anyPunch or anyKick
    local attackPressed = attackHeld and not self.attackWasHeld
    self.attackWasHeld = attackHeld
    if self.attackTimer > 0.0 then
        -- committed attack keeps going even if the key is released
        anyPunch, anyKick = self.latchPunch == true, self.latchKick == true
    elseif attackPressed and self.attackCd <= 0.0 then
        self.latchPunch, self.latchKick = anyPunch, anyKick
        self.attackTimer = ATTACK_ACTIVE
        self.attackCd = ATTACK_ACTIVE + ATTACK_DELAY
    else
        anyPunch, anyKick = false, false
    end

    -- hadouken: only fighters whose prefab has a "hadouken" clip (Ryu)
    if canAct and self.isGrounded and not crouchActive and self.fireCooldown <= 0.0
        and self:isAction("Hadouken")
        and Animation and Animation.play(self.entity, "hadouken", true) then
        self.requestedAnim, self.currentAnim, self.netAnim = "hadouken", "hadouken", "hadouken"
        self.castTimer, self.castFired = CAST_TIME, false
        self.fireCooldown = CAST_COOLDOWN
        casting = true
        canAct = false
        leftActive, rightActive, jumpActive = false, false, false
        anyPunch, anyKick = false, false
    end

    -- blocking: grounded, standing still, no attack pressed
    local isBlocking = blockActive and self.isGrounded and not anyPunch and not anyKick
    FighterBlocking[self.playerId] = isBlocking
    if isBlocking then
        leftActive, rightActive, jumpActive = false, false, false
    end

    if self.isGrounded then
        self.isCrouching = crouchActive
    end

    local jumpPressed = jumpActive and not self.jumpHeld
    self.jumpHeld = jumpActive
    self.airJumped = false

    if self.isGrounded and jumpActive and not self.isCrouching then
        self.vy = self.jumpForce
        self.isGrounded = false
        self.jumpsUsed = 1
        self.jumpDirectional = (leftActive ~= rightActive)
    elseif not self.isGrounded and jumpPressed and self.jumpsUsed < self.maxJumps then
        self.vy = self.doubleJumpForce
        self.jumpsUsed = self.jumpsUsed + 1
        self.jumpDirectional = (leftActive ~= rightActive)
        self.airJumped = true
    end

    local dx = 0.0
    local isMovingHorizontally = false

    if not (self.isGrounded and self.isCrouching) then
        if leftActive and not rightActive then
            dx = -self.moveSpeed * dt
            isMovingHorizontally = true
            self.facingRight = false
        elseif rightActive and not leftActive then
            dx = self.moveSpeed * dt
            isMovingHorizontally = true
            self.facingRight = true
        end
    end

    self:applyFlip()

    local dy = 0.0
    if not self.isGrounded then
        self.vy = self.vy + self.gravity * dt
        dy = self.vy * dt
    end

    local newX = pos.x + dx
    local newY = pos.y + dy

    if newY >= self.groundY then
        newY = self.groundY
        self.vy = 0.0
        self.isGrounded = true
        self.jumpsUsed = 0
    end

    Transform.setPosition(entity, newX, newY, pos.z)

    local isAttacking = false

    if stunned then
        local seq = FighterHitSeq[self.playerId] or 0
        local restart = seq ~= self.lastHitSeq
        self.lastHitSeq = seq
        self:playAnim("hit", "crouch", restart)
    elseif casting then
        self:playAnim("hadouken")
    elseif not self.isGrounded then
        if anyKick then
            self:playAnim("jump_kick")
            self:setHitboxActive(true, 45.0, 10.0, 65.0, 45.0)
            isAttacking = true
        elseif anyPunch then
            self:playAnim("jump_punch", "jump_kick")
            self:setHitboxActive(true, 45.0, 0.0, 60.0, 40.0)
            isAttacking = true
        elseif self.jumpDirectional and self.jumpsUsed >= 2 then
            -- the somersault only plays on the air (second) jump
            self:playAnim("jump_forward", "jump", self.airJumped)
        else
            self:playAnim("jump", nil, self.airJumped)
        end
    elseif isBlocking then
        self:playAnim("block", "crouch")
    elseif self.isCrouching then
        if anyPunch then
            self:playAnim("crouch_punch")
            self:setHitboxActive(true, 50.0, 15.0, 65.0, 35.0)
            isAttacking = true
        elseif anyKick then
            self:playAnim("low_kick")
            self:setHitboxActive(true, 55.0, 30.0, 70.0, 30.0)
            isAttacking = true
        else
            self:playAnim("crouch")
        end
    elseif anyPunch then
        self:playAnim("punch")
        self:setHitboxActive(true, 45.0, -15.0, 60.0, 40.0)
        isAttacking = true
    elseif anyKick then
        self:playAnim("kick")
        self:setHitboxActive(true, 50.0, 5.0, 65.0, 45.0)
        isAttacking = true
    elseif isMovingHorizontally then
        self:playAnim("Walk")
    else
        self:playAnim("Idle")
    end

    if not isAttacking then
        self:setHitboxActive(false)
    end
end

function Script:onDestroy(entity)
    log.info("[" .. tostring(entity) .. "] Fighter movement destroyed")
end

return Script
