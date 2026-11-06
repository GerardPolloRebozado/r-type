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
        HeavyKick = 76
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
        HeavyKick = 320
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
    self.gravity = 980.0

    self.vy = 0.0
    self.isGrounded = true
    self.isCrouching = false
    self.groundY = nil
    self.currentAnim = ""
    self.facingRight = (self.playerId == 1)
    self.hitboxEntity = nil

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
    if Controller and Controller.isActionActive then
        return Controller.isActionActive(actionName, self.playerId)
    end
    return false
end

function Script:playAnim(clipName, restartIfSame)
    if not Animation or not Animation.play then
        return
    end
    if self.currentAnim ~= clipName or restartIfSame then
        Animation.play(self.entity, clipName, restartIfSame or false)
        self.currentAnim = clipName
        self.netAnim = clipName
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

    if not self.groundY then
        self.groundY = pos.y
    end

    local leftActive = self:isAction("Left")
    local rightActive = self:isAction("Right")
    local jumpActive = self:isAction("Jump")
    local crouchActive = self:isAction("Crouch")

    local lightPunch = self:isAction("LightPunch")
    local mediumPunch = self:isAction("MediumPunch")
    local heavyPunch = self:isAction("HeavyPunch")
    local anyPunch = lightPunch or mediumPunch or heavyPunch

    local lightKick = self:isAction("LightKick")
    local mediumKick = self:isAction("MediumKick")
    local heavyKick = self:isAction("HeavyKick")
    local anyKick = lightKick or mediumKick or heavyKick

    if self.isGrounded then
        self.isCrouching = crouchActive
    end

    if self.isGrounded and jumpActive and not self.isCrouching then
        self.vy = self.jumpForce
        self.isGrounded = false
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
    end

    Transform.setPosition(entity, newX, newY, pos.z)

    local isAttacking = false

    if not self.isGrounded then
        if anyKick then
            self:playAnim("jump_kick")
            self:setHitboxActive(true, 45.0, 10.0, 65.0, 45.0)
            isAttacking = true
        else
            self:playAnim("jump")
        end
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
