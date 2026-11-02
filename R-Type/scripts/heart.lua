local Heart = {}
Heart.entities = {}
Heart.maxLives = nil
Heart.lastPlayerLives = nil
Heart.SPACING = 50
Heart.STARTING_X = -1300.000
Heart.STARTING_Y = -500.000

local function maxLives()
    if Player ~= nil and Player.MAX_LIVES ~= nil then
        return Player.MAX_LIVES
    end
    return 3
end

local function playerEntity()
    if Net ~= nil and Net.localEntity ~= nil then
        local entity = Net.localEntity()
        if entity ~= nil and Player ~= nil and Player.states[entity] ~= nil then
            return entity
        end
    end
    if Player ~= nil and Player.states ~= nil then
        for entity, state in pairs(Player.states) do
            if state ~= nil then
                return entity
            end
        end
    end
    return Entity.find("Player")
end

local function playerLives()
    local entity = playerEntity()
    if entity == nil or Player == nil or Player.states == nil then
        return nil
    end
    local state = Player.states[entity]
    if state == nil then
        return nil
    end
    return state.lives
end

function Heart:onCreate(entity)
    self.entity = entity
    Heart.maxLives = maxLives()
    Heart.lastPlayerLives = Heart.maxLives
    log.info("[" .. tostring(entity) .. "] Script created")
    for i = 1, Heart.maxLives do
        Heart:createHeart(entity, i)
    end
end

function Heart:createHeart(entity, index)
    local heart = Entity.create()
    Entity.setName(heart, "Heart" .. index)
    Component.addSprite(heart, "assets/heart.png")
    Component.setSpriteScale(heart, 0.2, 0.2)
    Component.setParent(heart, entity, true)
    Transform.setPosition(heart, Heart.STARTING_X + (index * Heart.SPACING), Heart.STARTING_Y)
    Component.setOpacity(heart, 1.0)
    Heart.entities[index] = heart
end

function Heart:syncHearts(entity, lives)
    for i = Heart.maxLives, 1, -1 do
        if i > lives then
            if Heart.entities[i] ~= nil then
                Entity.destroy(Heart.entities[i])
                Heart.entities[i] = nil
            end
        elseif Heart.entities[i] == nil then
            Heart:createHeart(entity, i)
        end
    end
end

function Heart:onUpdate(entity, dt)
    local lives = playerLives()
    if lives == nil then
        return
    end
    if Heart.lastPlayerLives == nil then
        Heart.lastPlayerLives = lives
        return
    end
    if lives == Heart.lastPlayerLives then
        return
    end
    Heart:syncHearts(entity, lives)
    Heart.lastPlayerLives = lives
end

function Heart:onDestroy(entity)
    log.info("[" .. tostring(entity) .. "] Heart destroyed")
end

return Heart
