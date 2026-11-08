-- game_rules.lua
-- Openfront game rules defined in Lua, demonstrating engine scripting capability.
-- These rules are loaded by the engine's scripting system and called from C++.

-- Building costs
building_costs = {
    City = 40000,
    Port = 35000,
    Factory = 30000,
    DefensePost = 25000,
    SamLauncher = 45000,
    MissileSilo = 75000,
}

-- Bomb costs
bomb_costs = {
    AtomBomb = 100000,
    HydrogenBomb = 250000,
}

-- Combat constants
combat = {
    defender_shield_multiplier = 10.0,
    attacker_loss_clamp_min = 0.6,
    attacker_loss_clamp_max = 2.0,
    defense_post_radius = 30,
    defense_post_multiplier = 5.0,
}

-- Economy constants
economy = {
    base_income = 1000,
    port_trade_ship_income = 600,
    factory_train_income = 800,
    city_population_bonus = 25000,
}

-- Terrain multipliers
terrain = {
    plains = 1.0,
    highlands = 1.5,
    mountains = 2.5,
}

-- Building level costs (cost scales with level)
building_level_multipliers = {
    city = 1.5,
    port = 1.5,
    factory = 1.5,
}

-- Functions
function get_terrain_multiplier(terrain_type)
    return terrain[terrain_type] or 1.0
end

function get_building_level_cost(building_type, level)
    local base = building_costs[building_type] or 0
    local mult = building_level_multipliers[string.lower(building_type)] or 1.0
    return math.floor(base * (mult ^ (level - 1)))
end

function get_max_population(tiles_owned, city_levels_sum)
    local base_pop = (tiles_owned ^ 0.6) * 1000 + 50000
    local city_bonus = city_levels_sum * 25000
    return 2 * base_pop + city_bonus
end

-- Functions
function get_building_cost(building_type)
    return building_costs[building_type] or 0
end

function get_bomb_cost(bomb_type)
    return bomb_costs[bomb_type] or 0
end

function calculate_defender_shield(troops, tiles)
    if tiles <= 0 then return 0 end
    local density = troops / tiles
    return density * combat.defender_shield_multiplier
end

function calculate_attacker_loss(base_loss, troop_ratio, terrain_mult, has_defense_post)
    local clamped = math.max(combat.attacker_loss_clamp_min, math.min(combat.attacker_loss_clamp_max, troop_ratio))
    local mult = terrain_mult
    if has_defense_post then
        mult = mult * combat.defense_post_multiplier
    end
    return base_loss * clamped * mult
end

function get_base_income()
    return economy.base_income
end

function get_port_income()
    return economy.port_trade_ship_income
end

function get_factory_income()
    return economy.factory_train_income
end

function get_city_population_bonus()
    return economy.city_population_bonus
end
