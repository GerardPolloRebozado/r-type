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

    self.minX = 40.0
    self.maxX = 800.0

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

    if self.isGrounded then
        if crouchActive then
            self.isCrouching = true
        else
            self.isCrouching = false
        end
    end

    if self.isGrounded and jumpActive and not self.isCrouching then
        self.vy = self.jumpForce
        self.isGrounded = false
    end

    local dx = 0.0
    if not (self.isGrounded and self.isCrouching) then
        if leftActive and not rightActive then
            dx = -self.moveSpeed * dt
            if Component and Component.setSpriteFlip then
                Component.setSpriteFlip(entity, true)
            end
        elseif rightActive and not leftActive then
            dx = self.moveSpeed * dt
            if Component and Component.setSpriteFlip then
                Component.setSpriteFlip(entity, false)
            end
        end
    end

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

    if self.minX and newX < self.minX then
        newX = self.minX
    elseif self.maxX and newX > self.maxX then
        newX = self.maxX
    end

    Transform.setPosition(entity, newX, newY, pos.z)
end

function Script:onDestroy(entity)
    log.info("[" .. tostring(entity) .. "] Ryu Movement script destroyed")
end

return Script
