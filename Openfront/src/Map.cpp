#include "Map.hpp"
#include "Building.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace Openfront {

namespace {

inline float Smoothstep(float edge0, float edge1, float x) {
    x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

// 2D pseudo-random hash
inline float Hash2D(int x, int y, uint32_t seed) {
    uint32_t h =
        seed ^ (static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(y) * 668265263u);
    h = (h ^ (h >> 13)) * 1274126177u;
    return static_cast<float>(h ^ (h >> 16)) / 4294967295.0f;
}

float ValueNoise2D(float x, float y, uint32_t seed) {
    int xi = static_cast<int>(std::floor(x));
    int yi = static_cast<int>(std::floor(y));
    float xf = x - static_cast<float>(xi);
    float yf = y - static_cast<float>(yi);

    float sx = xf * xf * (3.0f - 2.0f * xf);
    float sy = yf * yf * (3.0f - 2.0f * yf);

    float v00 = Hash2D(xi, yi, seed);
    float v10 = Hash2D(xi + 1, yi, seed);
    float v01 = Hash2D(xi, yi + 1, seed);
    float v11 = Hash2D(xi + 1, yi + 1, seed);

    float ix0 = v00 + sx * (v10 - v00);
    float ix1 = v01 + sx * (v11 - v01);
    return ix0 + sy * (ix1 - ix0);
}

float Fbm2D(float x, float y, uint32_t seed, int octaves = 4, float persistence = 0.5f) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        total += ValueNoise2D(x * frequency, y * frequency, seed + i * 101) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    return total / maxValue;
}

} // namespace

WorldMap::WorldMap(int width, int height) : width_(width), height_(height), tiles_(width * height) {
    ComputeLandCount();
}

void WorldMap::Generate(uint32_t seed) {
    tiles_.assign(width_ * height_, Tile{});

    float centerX = static_cast<float>(width_) * 0.5f;
    float centerY = static_cast<float>(height_) * 0.5f;
    float maxDist = std::sqrt(centerX * centerX + centerY * centerY);

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            float nx = static_cast<float>(x) * 0.035f;
            float ny = static_cast<float>(y) * 0.035f;

            float elevation = Fbm2D(nx, ny, seed, 5, 0.5f);

            float dx = (static_cast<float>(x) - centerX) / centerX;
            float dy = (static_cast<float>(y) - centerY) / centerY;
            float edgeDist = std::sqrt(dx * dx + dy * dy);
            float edgeFalloff = 1.0f - std::clamp(std::pow(edgeDist, 2.5f), 0.0f, 1.0f);

            elevation *= (0.3f + 0.7f * edgeFalloff);

            // Water border guarantee
            if (x < 2 || x >= width_ - 2 || y < 2 || y >= height_ - 2) {
                elevation = 0.1f;
            }

            Tile& tile = GetTile(x, y);
            tile.ownerId = kNeutralPlayerId;
            tile.building = BuildingType::None;
            tile.buildingLevel = 0;
            tile.falloutTimer = 0.0f;
            tile.hasDefenseBonus = false;

            if (elevation < 0.35f) {
                tile.terrain = TerrainType::Water;
            } else if (elevation < 0.52f) {
                tile.terrain = TerrainType::Plains;
            } else if (elevation < 0.62f) {
                tile.terrain = TerrainType::Highlands;
            } else {
                tile.terrain = TerrainType::Mountains;
            }
        }
    }

    ComputeLandCount();
    UpdateBorders();
}

void WorldMap::ComputeLandCount() {
    landTilesCount_ = 0;
    for (const auto& t : tiles_) {
        if (t.terrain != TerrainType::Water) {
            landTilesCount_++;
        }
    }
}

void WorldMap::SetOwner(int x, int y, PlayerId owner) {
    if (!InBounds(x, y)) {
        return;
    }
    Tile& t = GetTile(x, y);
    if (t.terrain != TerrainType::Water) {
        t.ownerId = owner;
    }
}

void WorldMap::SetBuilding(int x, int y, BuildingType type, int level) {
    if (!InBounds(x, y)) {
        return;
    }
    Tile& t = GetTile(x, y);
    t.building = type;
    t.buildingLevel = level;
}

void WorldMap::RemoveBuilding(int x, int y) {
    if (!InBounds(x, y)) {
        return;
    }
    Tile& t = GetTile(x, y);
    t.building = BuildingType::None;
    t.buildingLevel = 0;
}

bool WorldMap::IsCoastal(int x, int y) const {
    if (!InBounds(x, y)) {
        return false;
    }
    if (GetTile(x, y).terrain == TerrainType::Water) {
        return false;
    }

    static const int dx[] = {0, 0, -1, 1};
    static const int dy[] = {-1, 1, 0, 0};

    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (InBounds(nx, ny) && GetTile(nx, ny).terrain == TerrainType::Water) {
            return true;
        }
    }
    return false;
}

bool WorldMap::HasAdjacentOwner(int x, int y, PlayerId owner) const {
    if (!InBounds(x, y)) {
        return false;
    }

    static const int dx[] = {0, 0, -1, 1};
    static const int dy[] = {-1, 1, 0, 0};

    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (InBounds(nx, ny) && GetTile(nx, ny).ownerId == owner) {
            return true;
        }
    }
    return false;
}

std::vector<TileCoord> WorldMap::GetNeighbors4(int x, int y) const {
    std::vector<TileCoord> res;
    res.reserve(4);
    static const int dx[] = {0, 0, -1, 1};
    static const int dy[] = {-1, 1, 0, 0};

    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (InBounds(nx, ny)) {
            res.push_back({nx, ny});
        }
    }
    return res;
}

std::vector<TileCoord> WorldMap::GetNeighbors8(int x, int y) const {
    std::vector<TileCoord> res;
    res.reserve(8);
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            int nx = x + dx;
            int ny = y + dy;
            if (InBounds(nx, ny)) {
                res.push_back({nx, ny});
            }
        }
    }
    return res;
}

void WorldMap::UpdateDefenseBonuses(const std::vector<Building>& buildings) {
    // Reset all defense bonuses
    for (auto& t : tiles_) {
        t.hasDefenseBonus = false;
    }

    for (const auto& b : buildings) {
        if (!b.active || b.type != BuildingType::DefensePost) {
            continue;
        }

        int rad = static_cast<int>(b.range);
        int minX = std::max(0, b.x - rad);
        int maxX = std::min(width_ - 1, b.x + rad);
        int minY = std::max(0, b.y - rad);
        int maxY = std::min(height_ - 1, b.y + rad);
        float radSq = b.range * b.range;

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                Tile& t = GetTile(x, y);
                if (t.ownerId == b.ownerId) {
                    float distSq =
                        static_cast<float>((x - b.x) * (x - b.x) + (y - b.y) * (y - b.y));
                    if (distSq <= radSq) {
                        t.hasDefenseBonus = true;
                    }
                }
            }
        }
    }
}

void WorldMap::UpdateBorders() {
    static const int dx[] = {0, 0, -1, 1};
    static const int dy[] = {-1, 1, 0, 0};

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            Tile& t = GetTile(x, y);
            t.isBorder = false;
            if (t.terrain == TerrainType::Water || t.ownerId == kNeutralPlayerId) {
                continue;
            }

            for (int i = 0; i < 4; ++i) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                if (!InBounds(nx, ny)) {
                    t.isBorder = true;
                    break;
                }
                const Tile& nt = GetTile(nx, ny);
                if (nt.terrain == TerrainType::Water || nt.ownerId != t.ownerId) {
                    t.isBorder = true;
                    break;
                }
            }
        }
    }
}

void WorldMap::Tick(float dt) {
    for (auto& t : tiles_) {
        if (t.falloutTimer > 0.0f) {
            t.falloutTimer = std::max(0.0f, t.falloutTimer - dt);
        }
        if (t.captureFlashTimer > 0.0f) {
            t.captureFlashTimer = std::max(0.0f, t.captureFlashTimer - dt);
        }
    }
}

} // namespace Openfront
