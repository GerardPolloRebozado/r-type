#ifndef OPENFRONT_COMMON_HPP_
#define OPENFRONT_COMMON_HPP_

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <raylib.h>
#include <string>
#include <vector>

namespace Openfront {

using PlayerId = uint16_t;
constexpr PlayerId kNeutralPlayerId = 0;

constexpr int kMapWidth = 160;
constexpr int kMapHeight = 100;
constexpr float kTileSize = 16.0f; // 16x16 pixels per tile

constexpr float kSimulationTickRate = 10.0f; // 10 Hz
constexpr float kSimulationTickDt = 0.1f;    // 100 ms per tick

struct TileCoord {
    int x{0};
    int y{0};

    bool operator==(const TileCoord& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const TileCoord& other) const {
        return !(*this == other);
    }
};

enum class TerrainType : uint8_t { Plains = 0, Highlands = 1, Mountains = 2, Water = 3 };

enum class BuildingType : uint8_t {
    None = 0,
    City,
    Port,
    Factory,
    DefensePost,
    SamLauncher,
    MissileSilo
};

enum class FactionType : uint8_t { Human = 0, Nation = 1, Bot = 2 };

enum class BombType : uint8_t { AtomBomb, HydrogenBomb };

inline const char* BuildingName(BuildingType type) {
    switch (type) {
        case BuildingType::City:
            return "City";
        case BuildingType::Port:
            return "Port";
        case BuildingType::Factory:
            return "Factory";
        case BuildingType::DefensePost:
            return "Defense Post";
        case BuildingType::SamLauncher:
            return "SAM Launcher";
        case BuildingType::MissileSilo:
            return "Missile Silo";
        default:
            return "None";
    }
}

inline double BuildingCost(BuildingType type, int level = 1) {
    switch (type) {
        case BuildingType::City:
            return 40000.0 * level;
        case BuildingType::Port:
            return 35000.0 * level;
        case BuildingType::Factory:
            return 30000.0 * level;
        case BuildingType::DefensePost:
            return 25000.0 * level;
        case BuildingType::SamLauncher:
            return 45000.0 * level;
        case BuildingType::MissileSilo:
            return 75000.0 * level;
        default:
            return 0.0;
    }
}

inline double BombCost(BombType type) {
    switch (type) {
        case BombType::AtomBomb:
            return 100000.0;
        case BombType::HydrogenBomb:
            return 250000.0;
    }
    return 100000.0;
}

inline float BombRadius(BombType type) {
    switch (type) {
        case BombType::AtomBomb:
            return 8.0f;
        case BombType::HydrogenBomb:
            return 16.0f;
    }
    return 8.0f;
}

inline float TerrainCostMultiplier(TerrainType t) {
    switch (t) {
        case TerrainType::Plains:
            return 1.0f;
        case TerrainType::Highlands:
            return 1.5f;
        case TerrainType::Mountains:
            return 2.5f;
        case TerrainType::Water:
            return 999.0f;
    }
    return 1.0f;
}

// Distance helper
inline float TileDistance(int x1, int y1, int x2, int y2) {
    float dx = static_cast<float>(x1 - x2);
    float dy = static_cast<float>(y1 - y2);
    return std::sqrt(dx * dx + dy * dy);
}

} // namespace Openfront

#endif // OPENFRONT_COMMON_HPP_
