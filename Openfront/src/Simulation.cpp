#include "Simulation.hpp"
#include "Components/Name.hpp"
#include "Components/Position.hpp"
#include "ECS/Registry.hpp"
#include "OpenfrontComponents.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <queue>
#include <random>
#include <raymath.h>

namespace Openfront {

Simulation::Simulation() {
    Init();
}

void Simulation::Init(int numNations, int numBots, uint32_t seed) {
    map_.Generate(seed);
    players_.clear();
    buildings_.clear();
    attackFronts_.clear();
    missiles_.clear();
    explosions_.clear();
    beams_.clear();
    clashParticles_.clear();
    floatingTexts_.clear();
    matchTime_ = 0.0f;
    currentTurn_ = 0;
    gameOver_ = false;
    winnerId_ = kNeutralPlayerId;
    nextFrontId_ = 1;
    nextMissileId_ = 1;

    const std::vector<Color> nationColors = {
        Color{230, 45, 50, 255},  // Crimson
        Color{34, 180, 70, 255},  // Forest Green
        Color{245, 175, 25, 255}, // Gold/Amber
        Color{160, 50, 220, 255}, // Royal Purple
        Color{235, 75, 140, 255}, // Rose
        Color{60, 180, 225, 255}, // Cyan
    };

    const std::vector<Color> botColors = {
        Color{130, 190, 40, 255},  // Lime
        Color{240, 110, 30, 255},  // Orange
        Color{175, 120, 220, 255}, // Violet
        Color{40, 190, 170, 255},  // Teal
        Color{210, 160, 100, 255}, // Brown
        Color{160, 160, 160, 255}  // Silver
    };

    const std::vector<std::string> nationNames = {"Aethelgard", "Novaria", "Valerius",
                                                  "Zephyria",   "Solaria", "Hyperion"};

    const std::vector<std::string> botNames = {"Bot Alpha", "Bot Beta",    "Bot Gamma",
                                               "Bot Delta", "Bot Epsilon", "Bot Zeta"};

    Player human;
    human.id = 1;
    human.name = "Commander (You)";
    human.type = FactionType::Human;
    human.color = Color{40, 140, 250, 255}; // Brilliant Electric Blue
    human.troops = 60000.0;
    human.gold = 25000.0;
    human.attackRatio = 0.20f;
    players_.push_back(human);

    for (int i = 0; i < numNations; ++i) {
        Player nation;
        nation.id = static_cast<PlayerId>(players_.size() + 1);
        nation.name = nationNames[i % nationNames.size()];
        nation.type = FactionType::Nation;
        nation.color = nationColors[i % nationColors.size()];
        nation.troops = 60000.0;
        nation.gold = 20000.0;
        nation.attackRatio = 0.20f;
        players_.push_back(nation);
    }

    for (int i = 0; i < numBots; ++i) {
        Player bot;
        bot.id = static_cast<PlayerId>(players_.size() + 1);
        bot.name = botNames[i % botNames.size()];
        bot.type = FactionType::Bot;
        bot.color = botColors[i % botColors.size()];
        bot.troops = 50000.0;
        bot.gold = 10000.0;
        bot.attackRatio = 0.20f;
        players_.push_back(bot);
    }

    int width = map_.GetWidth();
    int height = map_.GetHeight();
    std::vector<TileCoord> spawnLocations;

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> distX(15, width - 16);
    std::uniform_int_distribution<int> distY(15, height - 16);

    for (auto& player : players_) {
        bool placed = false;
        for (int attempts = 0; attempts < 500 && !placed; ++attempts) {
            int cx = distX(rng);
            int cy = distY(rng);

            if (map_.GetTile(cx, cy).terrain == TerrainType::Water) {
                continue;
            }

            // Check distance from existing spawns
            bool tooClose = false;
            for (const auto& sp : spawnLocations) {
                if (TileDistance(cx, cy, sp.x, sp.y) < 22.0f) {
                    tooClose = true;
                    break;
                }
            }
            if (tooClose) {
                continue;
            }

            // Spawn base territory
            SpawnPlayerBase(player.id, cx, cy, 3);
            spawnLocations.push_back({cx, cy});
            placed = true;
        }
    }

    map_.UpdateBorders();
    map_.UpdateDefenseBonuses(buildings_);
    RecalculateTerritoryAndBuildings();
    SyncToEcs();
}

void Simulation::SpawnPlayerBase(PlayerId id, int centerX, int centerY, int radius) {
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            int x = centerX + dx;
            int y = centerY + dy;
            if (!map_.InBounds(x, y)) {
                continue;
            }

            if (dx * dx + dy * dy <= radius * radius) {
                Tile& t = map_.GetTile(x, y);
                if (t.terrain != TerrainType::Water) {
                    t.ownerId = id;
                }
            }
        }
    }

    // Capital City at center
    if (map_.InBounds(centerX, centerY) &&
        map_.GetTile(centerX, centerY).terrain != TerrainType::Water) {
        Build(id, centerX, centerY, BuildingType::City);
    }
}

void Simulation::ApplyIntent(const ClientIntent& intent) {
    switch (intent.type) {
        case IntentType::Attack:
            LaunchAttack(intent.playerId, intent.startX, intent.startY, intent.targetX,
                         intent.targetY, intent.ratio);
            break;
        case IntentType::Reinforce:
            ReinforceFront(intent.playerId, intent.frontId);
            break;
        case IntentType::Build:
            Build(intent.playerId, intent.startX, intent.startY, intent.buildingType);
            break;
        case IntentType::LaunchNuke:
            LaunchNuke(intent.playerId, intent.startX, intent.startY, intent.targetX,
                       intent.targetY, intent.bombType);
            break;
        case IntentType::SetAttackRatio: {
            Player* p = GetPlayer(intent.playerId);
            if (p) {
                p->attackRatio = intent.ratio;
            }
            break;
        }
    }
}

void Simulation::ExecuteTurn(const std::vector<ClientIntent>& intents, float dt) {
    for (const auto& intent : intents) {
        ApplyIntent(intent);
    }
    Tick(dt);
    currentTurn_++;
}

void Simulation::CollectAIIntents(std::vector<ClientIntent>& outIntents, float dt) {
    if (gameOver_) {
        return;
    }
    for (auto& p : players_) {
        p.UpdateAI(*this, outIntents, dt);
    }
}

void Simulation::CancelAllFrontsForPlayer(PlayerId playerId) {
    Player* p = GetPlayer(playerId);
    for (auto& f : attackFronts_) {
        if (f.attackerId == playerId && !f.finished) {
            if (p && f.troops > 0.0) {
                p->troops += f.troops;
            }
            f.finished = true;
        }
    }
}

bool Simulation::AssignPlayerSlot(PlayerId id, const std::string& name, FactionType type) {
    Player* p = GetPlayer(id);
    if (!p) {
        return false;
    }
    p->name = name;
    p->type = type;
    return true;
}

Player* Simulation::GetPlayer(PlayerId id) {
    for (auto& p : players_) {
        if (p.id == id) {
            return &p;
        }
    }
    return nullptr;
}

const Player* Simulation::GetPlayer(PlayerId id) const {
    for (const auto& p : players_) {
        if (p.id == id) {
            return &p;
        }
    }
    return nullptr;
}

Building* Simulation::GetBuildingAt(int x, int y) {
    for (auto& b : buildings_) {
        if (b.x == x && b.y == y && b.active) {
            return &b;
        }
    }
    return nullptr;
}

const Building* Simulation::GetBuildingAt(int x, int y) const {
    for (const auto& b : buildings_) {
        if (b.x == x && b.y == y && b.active) {
            return &b;
        }
    }
    return nullptr;
}

std::vector<TileCoord> Simulation::FindPath(int startX, int startY, int targetX, int targetY,
                                            bool allowWater) {
    if (!map_.InBounds(startX, startY) || !map_.InBounds(targetX, targetY)) {
        return {};
    }

    int width = map_.GetWidth();
    int height = map_.GetHeight();

    std::vector<int> dist(width * height, -1);
    std::vector<TileCoord> parent(width * height, {-1, -1});
    std::queue<TileCoord> q;

    dist[startY * width + startX] = 0;
    q.push({startX, startY});

    bool found = false;
    while (!q.empty()) {
        TileCoord curr = q.front();
        q.pop();

        if (curr.x == targetX && curr.y == targetY) {
            found = true;
            break;
        }

        auto nbrs = map_.GetNeighbors4(curr.x, curr.y);
        for (const auto& nb : nbrs) {
            int idx = nb.y * width + nb.x;
            if (dist[idx] != -1) {
                continue;
            }

            const Tile& nt = map_.GetTile(nb.x, nb.y);
            if (!allowWater && nt.terrain == TerrainType::Water) {
                continue;
            }

            dist[idx] = dist[curr.y * width + curr.x] + 1;
            parent[idx] = curr;
            q.push(nb);
        }
    }

    if (!found) {
        return {};
    }

    std::vector<TileCoord> path;
    TileCoord curr = {targetX, targetY};
    while (curr.x != startX || curr.y != startY) {
        path.push_back(curr);
        int idx = curr.y * width + curr.x;
        curr = parent[idx];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

void Simulation::SpawnFloatingText(Vector2 pos, const std::string& text, Color color) {
    FloatingText ft;
    ft.pos = pos;
    ft.text = text;
    ft.color = color;
    ft.duration = 1.2f;
    ft.age = 0.0f;
    floatingTexts_.push_back(ft);
}

void Simulation::SpawnClashParticles(Vector2 pos, Color color, int count) {
    for (int i = 0; i < count; ++i) {
        CombatClashParticle p;
        p.pos = pos;
        float angle = static_cast<float>(rand() % 360) * DEG2RAD;
        float speed = 25.0f + static_cast<float>(rand() % 60);
        p.vel = {std::cos(angle) * speed, std::sin(angle) * speed};
        p.color = color;
        p.lifetime = 0.35f + static_cast<float>(rand() % 20) * 0.01f;
        p.age = 0.0f;
        clashParticles_.push_back(p);
    }
}

bool Simulation::LaunchAttack(PlayerId playerId, int targetX, int targetY, float ratio) {
    return LaunchAttack(playerId, targetX, targetY, targetX, targetY, ratio);
}

bool Simulation::LaunchAttack(PlayerId playerId, int startX, int startY, int targetX, int targetY,
                              float ratio) {
    Player* p = GetPlayer(playerId);
    if (!p || !p->isAlive) {
        return false;
    }

    if (!map_.InBounds(targetX, targetY)) {
        return false;
    }
    const Tile& targetTile = map_.GetTile(targetX, targetY);
    if (targetTile.terrain == TerrainType::Water) {
        return false;
    }
    if (targetTile.ownerId == playerId) {
        return false;
    }

    if (ratio <= 0.0f) {
        ratio = p->attackRatio;
    }
    ratio = std::clamp(ratio, 0.05f, 1.0f);

    double troopsToSend = p->troops * ratio;
    if (troopsToSend < 200.0 || p->troops < 500.0) {
        return false;
    }

    // Find closest border tile of player to target
    int width = map_.GetWidth();
    int height = map_.GetHeight();
    float closestDist = 1e9f;
    TileCoord closestBorder{-1, -1};
    std::vector<TileCoord> borderSector;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Tile& t = map_.GetTile(x, y);
            if (t.ownerId == playerId && t.isBorder) {
                float d = TileDistance(x, y, targetX, targetY);
                if (d < closestDist) {
                    closestDist = d;
                    closestBorder = {x, y};
                }
                if (d <= 12.0f) {
                    borderSector.push_back({x, y});
                }
            }
        }
    }

    if (closestBorder.x < 0) {
        return false;
    }
    if (borderSector.empty()) {
        borderSector.push_back(closestBorder);
    }

    p->troops -= troopsToSend;

    FrontPush front;
    front.id = nextFrontId_++;
    front.attackerId = playerId;
    front.targetPlayerId = targetTile.ownerId;
    front.targetX = targetX;
    front.targetY = targetY;
    front.initialTroops = troopsToSend;
    front.troops = troopsToSend;
    front.rate = 14.0f; // tiles attempted per second
    front.progressAccumulator = 0.0f;
    front.tilesCaptured = 0;
    front.finished = false;
    front.frontCenter = {(closestBorder.x + 0.5f) * kTileSize,
                         (closestBorder.y + 0.5f) * kTileSize};
    front.activeBorderTiles = std::move(borderSector);

    attackFronts_.push_back(front);

    SpawnFloatingText(front.frontCenter, TextFormat("Front! (%.1fk)", troopsToSend / 1000.0),
                      p->color);
    return true;
}

bool Simulation::ReinforceFront(PlayerId playerId, uint32_t frontId) {
    Player* p = GetPlayer(playerId);
    if (!p || !p->isAlive) {
        return false;
    }

    for (auto& f : attackFronts_) {
        if (f.id == frontId && f.attackerId == playerId && !f.finished) {
            double addTroops = p->troops * p->attackRatio;
            if (addTroops >= 200.0) {
                p->troops -= addTroops;
                f.troops += addTroops;
                SpawnFloatingText(f.frontCenter, TextFormat("+%.1fk Troops", addTroops / 1000.0),
                                  p->color);
                return true;
            }
        }
    }
    return false;
}

bool Simulation::CancelFront(PlayerId playerId, uint32_t frontId) {
    for (auto& f : attackFronts_) {
        if (f.id == frontId && f.attackerId == playerId && !f.finished) {
            Player* p = GetPlayer(playerId);
            if (p && f.troops > 0.0) {
                p->troops += f.troops;
            }
            f.finished = true;
            SpawnFloatingText(f.frontCenter, "Front Dissolved", GRAY);
            return true;
        }
    }
    return false;
}

bool Simulation::Build(PlayerId playerId, int x, int y, BuildingType type) {
    Player* p = GetPlayer(playerId);
    if (!p || !p->isAlive) {
        return false;
    }
    if (!map_.InBounds(x, y)) {
        return false;
    }

    Tile& t = map_.GetTile(x, y);
    if (t.ownerId != playerId || t.terrain == TerrainType::Water) {
        return false;
    }
    if (t.building != BuildingType::None) {
        return false;
    }

    if (type == BuildingType::Port && !map_.IsCoastal(x, y)) {
        return false;
    }

    double cost = BuildingCost(type, 1);
    if (p->gold < cost) {
        return false;
    }

    p->gold -= cost;
    map_.SetBuilding(x, y, type, 1);

    Building b;
    b.type = type;
    b.ownerId = playerId;
    b.x = x;
    b.y = y;
    b.level = 1;
    b.timer = 0.0f;
    b.range = Building::DefaultRange(type, 1);
    b.active = true;
    buildings_.push_back(b);

    if (type == BuildingType::DefensePost) {
        map_.UpdateDefenseBonuses(buildings_);
    }

    RecalculateTerritoryAndBuildings();
    return true;
}

bool Simulation::LaunchNuke(PlayerId playerId, int siloX, int siloY, int targetX, int targetY,
                            BombType type) {
    Player* p = GetPlayer(playerId);
    if (!p || !p->isAlive) {
        return false;
    }
    if (!map_.InBounds(siloX, siloY) || !map_.InBounds(targetX, targetY)) {
        return false;
    }

    Building* silo = GetBuildingAt(siloX, siloY);
    if (!silo || silo->ownerId != playerId || silo->type != BuildingType::MissileSilo) {
        return false;
    }

    double cost = BombCost(type);
    if (p->gold < cost) {
        return false;
    }

    p->gold -= cost;

    ActiveMissile m;
    m.id = nextMissileId_++;
    m.ownerId = playerId;
    m.type = type;
    m.startPos = {(siloX + 0.5f) * kTileSize, (siloY + 0.5f) * kTileSize};
    m.currentPos = m.startPos;
    m.targetPos = {(targetX + 0.5f) * kTileSize, (targetY + 0.5f) * kTileSize};
    m.targetTile = {targetX, targetY};
    m.progress = 0.0f;
    m.speed = 0.20f; // fraction per second (5s flight time)
    m.blastRadius = BombRadius(type);
    m.intercepted = false;
    m.exploded = false;

    missiles_.push_back(m);
    return true;
}

void Simulation::Tick(float dt) {
    if (gameOver_) {
        return;
    }

    matchTime_ += dt;

    for (auto& p : players_) {
        p.TickEconomy(dt);
    }

    map_.Tick(dt);

    TickCombatFronts(dt);

    TickMissiles(dt);

    map_.UpdateBorders();
    RecalculateTerritoryAndBuildings();

    CheckWinConditions();

    SyncToEcs();
}

void Simulation::TickCombatFronts(float dt) {
    int width = map_.GetWidth();
    int height = map_.GetHeight();

    for (auto& front : attackFronts_) {
        if (front.finished) {
            continue;
        }

        Player* att = GetPlayer(front.attackerId);
        if (!att || !att->isAlive) {
            front.finished = true;
            continue;
        }

        front.progressAccumulator += front.rate * dt;
        int attempts = static_cast<int>(std::floor(front.progressAccumulator));
        front.progressAccumulator -= attempts;

        if (attempts <= 0) {
            continue;
        }

        std::vector<std::pair<float, TileCoord>> candidates;

        int minX = std::max(
            0, std::min(front.targetX, static_cast<int>(front.frontCenter.x / kTileSize)) - 12);
        int maxX = std::min(
            width - 1,
            std::max(front.targetX, static_cast<int>(front.frontCenter.x / kTileSize)) + 12);
        int minY = std::max(
            0, std::min(front.targetY, static_cast<int>(front.frontCenter.y / kTileSize)) - 12);
        int maxY = std::min(
            height - 1,
            std::max(front.targetY, static_cast<int>(front.frontCenter.y / kTileSize)) + 12);

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                const Tile& t = map_.GetTile(x, y);
                if (t.ownerId != front.attackerId && t.terrain != TerrainType::Water) {
                    if (map_.HasAdjacentOwner(x, y, front.attackerId)) {
                        float dist = TileDistance(x, y, front.targetX, front.targetY);
                        candidates.push_back({dist, {x, y}});
                    }
                }
            }
        }

        if (candidates.empty()) {
            att->troops += front.troops;
            front.finished = true;
            SpawnFloatingText(front.frontCenter, "Front Done", att->color);
            continue;
        }

        std::sort(candidates.begin(), candidates.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });

        int capturesThisTick = 0;
        for (const auto& [dist, coord] : candidates) {
            if (capturesThisTick >= attempts || front.troops <= 100.0) {
                break;
            }

            int tx = coord.x;
            int ty = coord.y;
            Tile& tile = map_.GetTile(tx, ty);
            if (tile.ownerId == front.attackerId) {
                continue;
            }

            Player* def = GetPlayer(tile.ownerId);
            double defTroops = (def && def->isAlive) ? def->troops : 1500.0;
            int defTiles = (def && def->isAlive) ? std::max(1, def->tileCount) : 1;
            double defDensity =
                (def && def->isAlive) ? (defTroops / static_cast<double>(defTiles)) : 30.0;

            double troopRatio = defTroops / std::max(1.0, front.troops);
            double clampedRatio = std::clamp(troopRatio, 0.6, 2.0);
            float terrainMult = TerrainCostMultiplier(tile.terrain);
            float defenseMult = tile.hasDefenseBonus ? 5.0f : 1.0f;

            double baseLoss = std::max(120.0, defDensity * 0.75);
            double attackerLoss = baseLoss * clampedRatio * terrainMult * defenseMult;
            double defenderLoss = std::max(50.0, defDensity);

            if (front.troops >= attackerLoss) {
                front.troops -= attackerLoss;
                if (def && def->isAlive) {
                    def->troops = std::max(0.0, def->troops - defenderLoss);
                }

                if (tile.building != BuildingType::None) {
                    Building* b = GetBuildingAt(tx, ty);
                    if (b) {
                        b->ownerId = front.attackerId;
                        SpawnFloatingText({(tx + 0.5f) * kTileSize, (ty + 0.5f) * kTileSize},
                                          TextFormat("%s Captured!", BuildingName(b->type)),
                                          att->color);
                    }
                }

                map_.SetOwner(tx, ty, front.attackerId);
                tile.captureFlashTimer = 0.35f; // Visual flash ripple!
                front.tilesCaptured++;
                capturesThisTick++;

                SpawnClashParticles({(tx + 0.5f) * kTileSize, (ty + 0.5f) * kTileSize}, att->color,
                                    4);
                front.frontCenter = {(tx + 0.5f) * kTileSize, (ty + 0.5f) * kTileSize};
            } else {
                if (def && def->isAlive) {
                    def->troops = std::max(0.0, def->troops - front.troops * 0.4);
                }
                front.troops = 0.0;
                front.finished = true;
                SpawnFloatingText(front.frontCenter, "Front Repelled!", RED);
                break;
            }
        }

        if (front.troops <= 100.0 && !front.finished) {
            front.finished = true;
            SpawnFloatingText(front.frontCenter, "Front Exhausted", GRAY);
        }
    }

    std::erase_if(attackFronts_, [](const FrontPush& f) { return f.finished; });
}

void Simulation::TickMissiles(float dt) {
    for (auto& m : missiles_) {
        if (m.exploded) {
            continue;
        }

        m.progress += m.speed * dt;

        Vector2 direct = {m.startPos.x + (m.targetPos.x - m.startPos.x) * m.progress,
                          m.startPos.y + (m.targetPos.y - m.startPos.y) * m.progress};
        m.currentPos = direct;

        for (const auto& b : buildings_) {
            if (b.active && b.type == BuildingType::SamLauncher && b.ownerId != m.ownerId) {
                Vector2 samPos = {(b.x + 0.5f) * kTileSize, (b.y + 0.5f) * kTileSize};
                float dist = Vector2Distance(samPos, m.currentPos);
                if (dist <= b.range * kTileSize) {
                    // Intercepted!
                    m.intercepted = true;
                    m.exploded = true;

                    // Interception beam
                    InterceptionBeam beam;
                    beam.samPos = samPos;
                    beam.missilePos = m.currentPos;
                    beam.duration = 0.4f;
                    beam.elapsed = 0.0f;
                    beam.finished = false;
                    beams_.push_back(beam);

                    // Airburst explosion
                    ExplosionEffect exp;
                    exp.pos = m.currentPos;
                    exp.maxRadius = 24.0f;
                    exp.currentRadius = 0.0f;
                    exp.duration = 0.8f;
                    exp.elapsed = 0.0f;
                    exp.color = SKYBLUE;
                    exp.finished = false;
                    explosions_.push_back(exp);
                    break;
                }
            }
        }

        if (m.intercepted) {
            continue;
        }

        // Reached destination!
        if (m.progress >= 1.0f) {
            m.exploded = true;

            // Ground detonation explosion
            ExplosionEffect exp;
            exp.pos = m.targetPos;
            exp.maxRadius = m.blastRadius * kTileSize;
            exp.currentRadius = 0.0f;
            exp.duration = 1.8f;
            exp.elapsed = 0.0f;
            exp.color = (m.type == BombType::HydrogenBomb) ? RED : ORANGE;
            exp.finished = false;
            explosions_.push_back(exp);

            // Nuclear fallout damage
            int rad = static_cast<int>(m.blastRadius);
            int minX = std::max(0, m.targetTile.x - rad);
            int maxX = std::min(map_.GetWidth() - 1, m.targetTile.x + rad);
            int minY = std::max(0, m.targetTile.y - rad);
            int maxY = std::min(map_.GetHeight() - 1, m.targetTile.y + rad);
            float radSq = m.blastRadius * m.blastRadius;

            for (int y = minY; y <= maxY; ++y) {
                for (int x = minX; x <= maxX; ++x) {
                    float dSq = static_cast<float>((x - m.targetTile.x) * (x - m.targetTile.x) +
                                                   (y - m.targetTile.y) * (y - m.targetTile.y));
                    if (dSq <= radSq) {
                        Tile& t = map_.GetTile(x, y);
                        if (t.terrain != TerrainType::Water) {
                            if (t.ownerId != kNeutralPlayerId) {
                                Player* victim = GetPlayer(t.ownerId);
                                if (victim) {
                                    victim->troops = std::max(0.0, victim->troops - 1500.0);
                                }
                            }
                            t.ownerId = kNeutralPlayerId;
                            t.falloutTimer = 45.0f;

                            // Destroy building
                            if (t.building != BuildingType::None) {
                                Building* b = GetBuildingAt(x, y);
                                if (b) {
                                    b->active = false;
                                }
                                t.building = BuildingType::None;
                                t.buildingLevel = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    std::erase_if(missiles_, [](const ActiveMissile& m) { return m.exploded; });
}

void Simulation::UpdateVisuals(float frameDt) {
    for (auto& exp : explosions_) {
        exp.elapsed += frameDt;
        exp.currentRadius = exp.maxRadius * (exp.elapsed / exp.duration);
        if (exp.elapsed >= exp.duration) {
            exp.finished = true;
        }
    }
    std::erase_if(explosions_, [](const ExplosionEffect& e) { return e.finished; });

    for (auto& b : beams_) {
        b.elapsed += frameDt;
        if (b.elapsed >= b.duration) {
            b.finished = true;
        }
    }
    std::erase_if(beams_, [](const InterceptionBeam& b) { return b.finished; });

    for (auto& p : clashParticles_) {
        p.pos.x += p.vel.x * frameDt;
        p.pos.y += p.vel.y * frameDt;
        p.age += frameDt;
    }
    std::erase_if(clashParticles_, [](const auto& p) { return p.age >= p.lifetime; });

    for (auto& ft : floatingTexts_) {
        ft.pos.y -= 25.0f * frameDt;
        ft.age += frameDt;
    }
    std::erase_if(floatingTexts_, [](const auto& ft) { return ft.age >= ft.duration; });
}

void Simulation::RecalculateTerritoryAndBuildings() {
    // Reset player counters
    for (auto& p : players_) {
        p.tileCount = 0;
        p.cityCount = 0;
        p.cityLevelsSum = 0;
        p.portCount = 0;
        p.factoryCount = 0;
        p.defensePostCount = 0;
        p.samCount = 0;
        p.siloCount = 0;
    }

    // Recount tiles
    int width = map_.GetWidth();
    int height = map_.GetHeight();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            PlayerId oid = map_.GetTile(x, y).ownerId;
            if (oid != kNeutralPlayerId) {
                Player* p = GetPlayer(oid);
                if (p) {
                    p->tileCount++;
                }
            }
        }
    }

    // Recount buildings
    for (const auto& b : buildings_) {
        if (!b.active) {
            continue;
        }
        Player* p = GetPlayer(b.ownerId);
        if (!p) {
            continue;
        }

        switch (b.type) {
            case BuildingType::City:
                p->cityCount++;
                p->cityLevelsSum += b.level;
                break;
            case BuildingType::Port:
                p->portCount++;
                break;
            case BuildingType::Factory:
                p->factoryCount++;
                break;
            case BuildingType::DefensePost:
                p->defensePostCount++;
                break;
            case BuildingType::SamLauncher:
                p->samCount++;
                break;
            case BuildingType::MissileSilo:
                p->siloCount++;
                break;
            default:
                break;
        }
    }

    // Check life status
    for (auto& p : players_) {
        if (p.isAlive && p.tileCount <= 0) {
            p.isAlive = false;
        }
    }
}

void Simulation::CheckWinConditions() {
    int aliveCount = 0;
    PlayerId lastAlive = kNeutralPlayerId;

    for (const auto& p : players_) {
        if (p.isAlive) {
            aliveCount++;
            lastAlive = p.id;
        }
    }

    if (aliveCount <= 1) {
        gameOver_ = true;
        winnerId_ = lastAlive;
    } else if (matchTime_ >= matchDuration_) {
        gameOver_ = true;
        // Winner is player with most territory
        int maxTiles = -1;
        for (const auto& p : players_) {
            if (p.isAlive && p.tileCount > maxTiles) {
                maxTiles = p.tileCount;
                winnerId_ = p.id;
            }
        }
    }
}

void Simulation::SyncToEcs() {
    if (!ecsRegistry_) {
        return;
    }

    // Clean up old entities
    for (size_t ent : ecsEntities_) {
        ecsRegistry_->DestroyEntity(ent);
    }
    ecsEntities_.clear();

    // Sync buildings to ECS
    for (const auto& b : buildings_) {
        size_t ent = ecsRegistry_->CreateEntity();
        // CreateEntity() always adds a default Position; update it via reference wrapper
        if (auto pos = ecsRegistry_->GetComponent<Engine::Position>(ent)) {
            pos->get() = Engine::Position(b.x * kTileSize, b.y * kTileSize, 1);
        }
        ecsRegistry_->AddComponent<Engine::Name>(
            ent, Engine::Name::Create(std::string(BuildingName(b.type)) + " (P" +
                                      std::to_string(b.ownerId) + ")"));
        ecsRegistry_->AddComponent<TacticalUnitComponent>(
            ent, TacticalUnitComponent{b.type, b.ownerId, b.level, b.x, b.y});
        ecsEntities_.push_back(ent);
    }

    // Sync active missiles to ECS
    for (const auto& m : missiles_) {
        if (m.exploded || m.intercepted) {
            continue;
        }
        size_t ent = ecsRegistry_->CreateEntity();
        if (auto pos = ecsRegistry_->GetComponent<Engine::Position>(ent)) {
            pos->get() =
                Engine::Position(m.currentPos.x * kTileSize, m.currentPos.y * kTileSize, 2);
        }
        ecsRegistry_->AddComponent<Engine::Name>(ent, Engine::Name::Create("ICBM Warhead"));
        ecsRegistry_->AddComponent<WarheadComponent>(
            ent, WarheadComponent{m.id, m.ownerId, m.type, m.progress, m.currentPos.x,
                                  m.currentPos.y, m.targetPos.x, m.targetPos.y});
        ecsEntities_.push_back(ent);
    }

    // Sync active front pushes to ECS
    for (const auto& f : attackFronts_) {
        if (f.finished) {
            continue;
        }
        size_t ent = ecsRegistry_->CreateEntity();
        if (auto pos = ecsRegistry_->GetComponent<Engine::Position>(ent)) {
            pos->get() = Engine::Position(f.frontCenter.x, f.frontCenter.y, 1);
        }
        ecsRegistry_->AddComponent<Engine::Name>(ent, Engine::Name::Create("Tactical Front Push"));
        ecsRegistry_->AddComponent<FrontLineComponent>(
            ent, FrontLineComponent{f.id, f.attackerId, f.targetPlayerId, f.troops, f.frontCenter.x,
                                    f.frontCenter.y});
        ecsEntities_.push_back(ent);
    }
}

} // namespace Openfront
