Heart = {}
Heart.maxLives = 3
Heart.SPACING = 50
Heart.STARTING_X = -850.0  -- near left edge of screen, visible
Heart.STARTING_Y = -400.0  -- near top edge of screen, visible
Heart.playerEntity = nil
Heart.lastLives = nil

function Heart:onCreate(entity)
    if Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked() and Net.isServer() then
        return
    end
    self.entity = entity
    self.playerEntity = nil
    self.lastLives = nil
    self.entities = {}
    log.info("[" .. tostring(entity) .. "] Heart script created")
    for i = 1, self.maxLives do
        self:createHeart(i)
    end
end

function Heart:createHeart(index)
    local heart = Entity.create()
    Entity.setName(heart, "Heart" .. index)
    -- addSprite accepts (entity, path, offsetX, offsetY, scaleX, scaleY)
    Component.addSprite(heart, "assets/heart.png", 0, 0, 0.2, 0.2)
    Component.setParent(heart, self.entity, true)
    Transform.setPosition(heart, Heart.STARTING_X + (index * Heart.SPACING), Heart.STARTING_Y)
    Component.setOpacity(heart, 1.0)
    self.entities[index] = heart
end

function Heart:syncHearts(lives)
    if Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked() and Net.isServer() then
        return
    end
    lives = math.max(0, math.min(self.maxLives, math.floor(lives or 0)))
    for i = self.maxLives, 1, -1 do
        if i > lives then
            if self.entities[i] ~= nil then
                Entity.destroy(self.entities[i])
                self.entities[i] = nil
            end
        elseif self.entities[i] == nil then
            self:createHeart(i)
        end
    end
end

function Heart:setLives(playerEntity, lives)
    if Net ~= nil and Net.isNetworked ~= nil and Net.isNetworked() and Net.isServer() then
        return
    end
    if self.playerEntity == nil then
        self.playerEntity = playerEntity
    elseif self.playerEntity ~= playerEntity then
        return
    end
    if lives == self.lastLives then
        return
    end
    self.lastLives = lives
    self:syncHearts(lives)
end

function Heart:onUpdate(entity, dt)
end

function Heart:onDestroy(entity)
    log.info("[" .. tostring(entity) .. "] Heart destroyed")
end

return Heart
