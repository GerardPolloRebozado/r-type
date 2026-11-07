local Script = {}

function Script:onCreate(entity)
    self.entity = entity
    log.info("[" .. tostring(entity) .. "] Ryu Movement script started")

    -- Movement parameters
    self.moveSpeed = 220.0       -- Horizontal walk speed
    self.jumpForce = -420.0      -- Initial vertical jump velocity
    self.gravity = 980.0         -- Gravity acceleration

    -- State flags
    self.vy = 0.0
    self.isGrounded = true
    self.isCrouching = false
    self.groundY = nil
    self.currentAnim = ""
    self.attackTimer = 0.0
    self.facingRight = true

    self.minX = 40.0
    self.maxX = 800.0


    self.hitboxEntity = nil
    self:setupKeybinds()
end

function Script:setupKeybinds()
    if not Controller or not Controller.addKeybind then
        return
    end

    Controller.addKeybind("Left", 65, Controller.KEYBOARD)
    Controller.addKeybind("Right", 68, Controller.KEYBOARD)
    Controller.addKeybind("Jump", 87, Controller.KEYBOARD)
    Controller.addKeybind("Crouch", 83, Controller.KEYBOARD)

    Controller.addKeybind("LightPunch", 85, Controller.KEYBOARD)
    Controller.addKeybind("MediumPunch", 73, Controller.KEYBOARD)
    Controller.addKeybind("HeavyPunch", 79, Controller.KEYBOARD)

    Controller.addKeybind("LightKick", 74, Controller.KEYBOARD)
    Controller.addKeybind("MediumKick", 75, Controller.KEYBOARD)
    Controller.addKeybind("HeavyKick", 76, Controller.KEYBOARD)
end

function Script:isAction(actionName)
    if Controller and Controller.isActionActive then
        return Controller.isActionActive(actionName, 1)
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
    end
end

function Script:findHitboxEntity()
    if self.hitboxEntity ~= nil then
        return self.hitboxEntity
    end
    if Component and Component.findEntityByName then
        local found = Component.findEntityByName("RyuHitbox")
        if found then
            self.hitboxEntity = found
            return self.hitboxEntity
        end
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
    if active then
        local dir = self.facingRight and 1 or -1
        Transform.setPosition(hb, (offsetX or 40.0) * dir, offsetY or 0.0, 0)
        if width and height and Component.setColliderSize then
            Component.setColliderSize(hb, width, height)
        end
    end
end

function Script:onUpdate(entity, dt)
    if dt <= 0 then return end

    local pos = Transform.getPosition(entity)
    if not pos then return end

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

    -- Jump logic
    if self.isGrounded and jumpActive and not self.isCrouching then
        self.vy = self.jumpForce
        self.isGrounded = false
    end

    -- Horizontal movement
    local dx = 0.0
    local isMovingHorizontally = false

    if not (self.isGrounded and self.isCrouching) then
        if leftActive and not rightActive then
            dx = -self.moveSpeed * dt
            isMovingHorizontally = true
            self.facingRight = false
            if Animation and Animation.setFlip then
                Animation.setFlip(entity, true)
            elseif Component and Component.setSpriteFlip then
                Component.setSpriteFlip(entity, true)
            end
        elseif rightActive and not leftActive then
            dx = self.moveSpeed * dt
            isMovingHorizontally = true
            self.facingRight = true
            if Animation and Animation.setFlip then
                Animation.setFlip(entity, false)
            elseif Component and Component.setSpriteFlip then
                Component.setSpriteFlip(entity, false)
            end
        end
    end

    -- Vertical movement
    local dy = 0.0
    if not self.isGrounded then
        self.vy = self.vy + self.gravity * dt
        dy = self.vy * dt
    end

    -- Apply position update with boundaries
    local newX = pos.x + dx
    local newY = pos.y + dy

    -- Ground collision check
    if newY >= self.groundY then
        newY = self.groundY
        self.vy = 0.0
        self.isGrounded = true
    end

    if self.minX and newX < self.minX then
        newX = self.minX
    elseif self.maxX and newX > self.maxX then
        newX = self.maxX
    end

    Transform.setPosition(entity, newX, newY, pos.z)

    -- Attack Hitbox & Animation State Machine
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
    log.info("[" .. tostring(entity) .. "] Ryu Movement script destroyed")
end

return Script
