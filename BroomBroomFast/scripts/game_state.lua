-- game_state.lua
-- Choices that must survive scene changes (the Lua state is kept between scenes).

GameState = GameState or {}
GameState.map = GameState.map or "Race_Asphalt.scene"
GameState.car = GameState.car or "red"

GameState.MAPS = {
    { scene = "Race_Asphalt.scene", label = "ASPHALT" },
    { scene = "Race_Dirt.scene", label = "DIRT" },
    { scene = "Race_Grass.scene", label = "GRASS" },
}
GameState.CARS = { "red", "blue", "green", "yellow", "black" }

function GameState.carSprite(color)
    return "assets/Cars/car_" .. color .. ".png"
end

return GameState
