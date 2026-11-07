#include "Player.hpp"
#include "Simulation.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace Openfront {

void Player::CalculateMaxTroops() {
    double basePop = std::pow(std::max(0, tileCount), 0.6) * 1000.0 + 50000.0;
    double cityBonus = static_cast<double>(cityLevelsSum) * 25000.0;
    maxTroops = 2.0 * basePop + cityBonus;
}

void Player::TickEconomy(float dt) {
    if (!isAlive) {
        return;
    }

    // Population regeneration
    CalculateMaxTroops();
    if (troops < maxTroops) {
        double diff = maxTroops - troops;
        double growthRate = 0.05; // 5% towards cap per sec
        troops = std::min(maxTroops, troops + diff * growthRate * dt + 500.0 * dt);
    }

    // Base gold income
    gold += 1000.0 * dt;

    // Bonus gold from Ports (Trade Ships) and Factories (Trains)
    if (portCount > 0) {
        gold += static_cast<double>(portCount) * 600.0 * dt;
    }
    if (factoryCount > 0) {
        gold += static_cast<double>(factoryCount) * 800.0 * dt;
    }
}

double Player::GetTroopDensity() const {
    if (tileCount <= 0) {
        return 0.0;
    }
    return troops / static_cast<double>(tileCount);
}

double Player::GetShield() const {
    return GetTroopDensity() * 10.0;
}

void Player::UpdateAI(Simulation& sim, std::vector<ClientIntent>& outIntents, float dt) {
    if (!isAlive || type == FactionType::Human) {
        return;
    }

    aiActionTimer -= dt;
    aiBuildTimer -= dt;
    aiNukeTimer -= dt;

    WorldMap& map = sim.GetMap();

    if (type == FactionType::Nation && aiBuildTimer <= 0.0f) {
        aiBuildTimer = 2.5f + static_cast<float>(rand() % 100) * 0.01f;

        // Try building infrastructure
        std::vector<TileCoord> myTiles;
        std::vector<TileCoord> myBorders;
        std::vector<TileCoord> myCoasts;

        int width = map.GetWidth();
        int height = map.GetHeight();

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const Tile& t = map.GetTile(x, y);
                if (t.ownerId == id) {
                    myTiles.push_back({x, y});
                    if (t.isBorder) {
                        myBorders.push_back({x, y});
                    }
                    if (map.IsCoastal(x, y)) {
                        myCoasts.push_back({x, y});
                    }
                }
            }
        }

        auto queueBuild = [&](int bx, int by, BuildingType bt) {
            ClientIntent it;
            it.playerId = id;
            it.type = IntentType::Build;
            it.startX = bx;
            it.startY = by;
            it.buildingType = bt;
            outIntents.push_back(it);
        };

        if (!myTiles.empty()) {
            // Defense Post near borders
            if (defensePostCount < 4 && gold >= BuildingCost(BuildingType::DefensePost) &&
                !myBorders.empty()) {
                TileCoord cand = myBorders[rand() % myBorders.size()];
                if (map.GetTile(cand.x, cand.y).building == BuildingType::None) {
                    queueBuild(cand.x, cand.y, BuildingType::DefensePost);
                }
            }
            // Cities for population
            else if (cityCount < 6 && gold >= BuildingCost(BuildingType::City)) {
                TileCoord cand = myTiles[rand() % myTiles.size()];
                if (map.GetTile(cand.x, cand.y).building == BuildingType::None) {
                    queueBuild(cand.x, cand.y, BuildingType::City);
                }
            }
            // Ports on coasts
            else if (portCount < 3 && gold >= BuildingCost(BuildingType::Port) &&
                     !myCoasts.empty()) {
                TileCoord cand = myCoasts[rand() % myCoasts.size()];
                if (map.GetTile(cand.x, cand.y).building == BuildingType::None) {
                    queueBuild(cand.x, cand.y, BuildingType::Port);
                }
            }
            // Factories for gold
            else if (factoryCount < 4 && gold >= BuildingCost(BuildingType::Factory)) {
                TileCoord cand = myTiles[rand() % myTiles.size()];
                if (map.GetTile(cand.x, cand.y).building == BuildingType::None) {
                    queueBuild(cand.x, cand.y, BuildingType::Factory);
                }
            }
            // SAM Launcher
            else if (samCount < 3 && gold >= BuildingCost(BuildingType::SamLauncher)) {
                TileCoord cand = myTiles[rand() % myTiles.size()];
                if (map.GetTile(cand.x, cand.y).building == BuildingType::None) {
                    queueBuild(cand.x, cand.y, BuildingType::SamLauncher);
                }
            }
            // Missile Silo
            else if (siloCount < 2 && gold >= BuildingCost(BuildingType::MissileSilo)) {
                TileCoord cand = myTiles[rand() % myTiles.size()];
                if (map.GetTile(cand.x, cand.y).building == BuildingType::None) {
                    queueBuild(cand.x, cand.y, BuildingType::MissileSilo);
                }
            }
        }
    }

    if (type == FactionType::Nation && siloCount > 0 && aiNukeTimer <= 0.0f) {
        aiNukeTimer = 15.0f + static_cast<float>(rand() % 150) * 0.1f;
        if (gold >= BombCost(BombType::AtomBomb)) {
            // Find our silo
            const Building* mySilo = nullptr;
            for (const auto& b : sim.GetBuildings()) {
                if (b.ownerId == id && b.type == BuildingType::MissileSilo && b.active) {
                    mySilo = &b;
                    break;
                }
            }

            if (mySilo) {
                // Find highest threat rival
                PlayerId rivalId = kNeutralPlayerId;
                double maxRivalTroops = 0.0;
                for (const auto& p : sim.GetPlayers()) {
                    if (p.id != id && p.isAlive && p.troops > maxRivalTroops) {
                        maxRivalTroops = p.troops;
                        rivalId = p.id;
                    }
                }

                if (rivalId != kNeutralPlayerId) {
                    // Pick a city or central tile of the rival
                    int targetX = -1;
                    int targetY = -1;
                    for (const auto& b : sim.GetBuildings()) {
                        if (b.ownerId == rivalId && b.type == BuildingType::City) {
                            targetX = b.x;
                            targetY = b.y;
                            break;
                        }
                    }

                    if (targetX < 0) {
                        // Pick random owned tile of rival
                        for (int y = 0; y < map.GetHeight(); ++y) {
                            for (int x = 0; x < map.GetWidth(); ++x) {
                                if (map.GetTile(x, y).ownerId == rivalId) {
                                    targetX = x;
                                    targetY = y;
                                    break;
                                }
                            }
                            if (targetX >= 0) {
                                break;
                            }
                        }
                    }

                    if (targetX >= 0) {
                        ClientIntent it;
                        it.playerId = id;
                        it.type = IntentType::LaunchNuke;
                        it.startX = mySilo->x;
                        it.startY = mySilo->y;
                        it.targetX = targetX;
                        it.targetY = targetY;
                        it.bombType = BombType::AtomBomb;
                        outIntents.push_back(it);
                    }
                }
            }
        }
    }

    if (aiActionTimer <= 0.0f) {
        aiActionTimer = (type == FactionType::Bot)
                            ? (1.5f + static_cast<float>(rand() % 100) * 0.01f)
                            : (1.0f + static_cast<float>(rand() % 80) * 0.01f);

        // Check if we have enough troops to attack
        if (troops > maxTroops * 0.20) {
            // Find border tiles
            std::vector<std::pair<TileCoord, TileCoord>> attackOptions;
            int width = map.GetWidth();
            int height = map.GetHeight();

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    const Tile& t = map.GetTile(x, y);
                    if (t.ownerId == id && t.isBorder) {
                        auto nbrs = map.GetNeighbors4(x, y);
                        for (const auto& nb : nbrs) {
                            const Tile& nt = map.GetTile(nb.x, nb.y);
                            if (nt.terrain != TerrainType::Water && nt.ownerId != id) {
                                attackOptions.push_back({{x, y}, nb});
                            }
                        }
                    }
                }
            }

            if (!attackOptions.empty()) {
                // Prefer neutral territory first
                std::pair<TileCoord, TileCoord> chosen =
                    attackOptions[rand() % attackOptions.size()];
                for (const auto& opt : attackOptions) {
                    if (map.GetTile(opt.second.x, opt.second.y).ownerId == kNeutralPlayerId) {
                        chosen = opt;
                        break;
                    }
                }

                float ratio = (type == FactionType::Bot) ? 0.20f : 0.25f;
                ClientIntent it;
                it.playerId = id;
                it.type = IntentType::Attack;
                it.startX = chosen.first.x;
                it.startY = chosen.first.y;
                it.targetX = chosen.second.x;
                it.targetY = chosen.second.y;
                it.ratio = ratio;
                outIntents.push_back(it);
            }
        }
    }
}

} // namespace Openfront
