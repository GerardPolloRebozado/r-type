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

// Continent mask for realistic maps
float ContinentMask(int x, int y, int width, int height, MapType type, uint32_t seed) {
    float cx = static_cast<float>(width) * 0.5f;
    float cy = static_cast<float>(height) * 0.5f;
    float nx = static_cast<float>(x) / static_cast<float>(width);
    float ny = static_cast<float>(y) / static_cast<float>(height);

    auto distFrom = [&](float px, float py, float rx, float ry) -> float {
        float dx = (static_cast<float>(x) - px) / rx;
        float dy = (static_cast<float>(y) - py) / ry;
        return std::sqrt(dx * dx + dy * dy);
    };

    auto coastline = [&](float base, float nx, float ny, uint32_t s) -> float {
        float noise = Fbm2D(nx * 4.0f, ny * 4.0f, s, 3, 0.4f);
        return base * (0.85f + 0.15f * noise);
    };

    switch (type) {
        case MapType::Europe: {
            float mask = 0.0f;
            // Main European landmass (France/Germany/Poland area)
            float d1 = distFrom(cx * 0.55f, cy * 0.9f, cx * 0.5f, cy * 0.7f);
            mask = std::max(mask, 1.0f - std::clamp(std::pow(d1, 1.5f), 0.0f, 1.0f));
            // Scandinavian peninsula (top)
            float d2 = distFrom(cx * 0.6f, cy * 0.3f, cx * 0.25f, cy * 0.35f);
            mask = std::max(mask, 1.0f - std::clamp(std::pow(d2, 1.6f), 0.0f, 1.0f));
            // Iberian peninsula (bottom-left)
            float d3 = distFrom(cx * 0.25f, cy * 1.15f, cx * 0.2f, cy * 0.25f);
            mask = std::max(mask, 1.0f - std::clamp(std::pow(d3, 1.6f), 0.0f, 1.0f));
            // Italian boot (bottom-center)
            float d4 = distFrom(cx * 0.7f, cy * 1.25f, cx * 0.12f, cy * 0.3f);
            mask = std::max(mask, 1.0f - std::clamp(std::pow(d4, 1.5f), 0.0f, 1.0f));
            // British Isles (top-left, separated by water)
            float d5 = distFrom(cx * 0.3f, cy * 0.5f, cx * 0.15f, cy * 0.2f);
            float island5 = 1.0f - std::clamp(std::pow(d5, 2.0f), 0.0f, 1.0f);
            if (x > static_cast<int>(cx * 0.45f) && x < static_cast<int>(cx * 0.55f) &&
                y > static_cast<int>(cy * 0.4f) && y < static_cast<int>(cy * 0.7f)) {
                island5 *= 0.1f;
            }
            mask = std::max(mask, island5);
            // Balkans (bottom-right)
            float d6 = distFrom(cx * 0.9f, cy * 1.05f, cx * 0.2f, cy * 0.3f);
            mask = std::max(mask, 1.0f - std::clamp(std::pow(d6, 1.6f), 0.0f, 1.0f));
            // Cut out Mediterranean sea
            if (x > static_cast<int>(cx * 0.7f) && y > static_cast<int>(cy * 0.8f)) {
                mask *= 0.15f;
            }
            // Cut out North Sea
            if (x > static_cast<int>(cx * 0.8f) && y < static_cast<int>(cy * 0.4f)) {
                mask *= 0.2f;
            }
            return coastline(mask, nx, ny, seed);
        }
        case MapType::World: {
            float mask = 0.0f;
            // North America
            float d1 = distFrom(cx * 0.2f, cy * 0.35f, cx * 0.25f, cy * 0.35f);
            mask = std::max(mask, 1.0f - std::clamp(std::pow(d1, 1.7f), 0.0f, 1.0f));
            // South America
            float d2 = distFrom(cx * 0.3f, cy * 1.1f, cx * 0.15f, cy * 0.4f);
            mask = std::max(mask, 1.0f - std::clamp(std::pow(d2, 1.7f), 0.0f, 1.0f));
            // Eurasia
            float d3 = distFrom(cx * 1.5f, cy * 0.4f, cx * 0.5f, cy * 0.4f);
            mask = std::max(mask, 1.0f - std::clamp(std::pow(d3, 1.6f), 0.0f, 1.0f));
            // Africa
            float d4 = distFrom(cx * 1.4f, cy * 1.1f, cx * 0.2f, cy * 0.4f);
            mask = std::max(mask, 1.0f - std::clamp(std::pow(d4, 1.7f), 0.0f, 1.0f));
            // Australia
            float d5 = distFrom(cx * 1.85f, cy * 1.3f, cx * 0.1f, cy * 0.15f);
            mask = std::max(mask, 1.0f - std::clamp(std::pow(d5, 2.0f), 0.0f, 1.0f));
            // Cut Atlantic ocean
            if (x > static_cast<int>(cx * 0.5f) && x < static_cast<int>(cx * 1.1f)) {
                mask *= 0.15f;
            }
            return coastline(mask, nx, ny, seed);
        }
        case MapType::Pangaea: {
            float dx = (static_cast<float>(x) - cx) / (cx * 0.9f);
            float dy = (static_cast<float>(y) - cy) / (cy * 0.85f);
            float dist = std::sqrt(dx * dx + dy * dy);
            float mask = 1.0f - std::clamp(std::pow(dist, 1.4f), 0.0f, 1.0f);
            float bay = Fbm2D(nx * 6.0f, ny * 6.0f, seed + 200, 4, 0.5f);
            mask *= (0.8f + 0.2f * bay);
            return coastline(mask, nx, ny, seed);
        }
        case MapType::Archipelago: {
            float noise = Fbm2D(nx * 3.0f, ny * 3.0f, seed + 500, 3, 0.6f);
            float mask = noise > 0.55f ? 1.0f : 0.0f;
            float d1 = distFrom(cx * 0.3f, cy * 0.5f, cx * 0.15f, cy * 0.2f);
            float d2 = distFrom(cx * 0.7f, cy * 0.4f, cx * 0.12f, cy * 0.18f);
            float d3 = distFrom(cx * 0.5f, cy * 0.8f, cx * 0.18f, cy * 0.25f);
            mask = std::max(mask, 1.0f - std::clamp(std::pow(d1, 2.0f), 0.0f, 1.0f));
            mask = std::max(mask, 1.0f - std::clamp(std::pow(d2, 2.0f), 0.0f, 1.0f));
            mask = std::max(mask, 1.0f - std::clamp(std::pow(d3, 2.0f), 0.0f, 1.0f));
            return mask;
        }
        default:
            return 1.0f;
    }
}

} // namespace

WorldMap::WorldMap(int width, int height) : width_(width), height_(height), tiles_(width * height) {
    ComputeLandCount();
}

void WorldMap::Generate(uint32_t seed, MapType type) {
    // Set map size based on type
    int newWidth = kMapWidth;
    int newHeight = kMapHeight;
    switch (type) {
        case MapType::Europe:
        case MapType::Pangaea:
        case MapType::Archipelago:
            newWidth = static_cast<int>(kMapWidth * 1.25f);
            newHeight = static_cast<int>(kMapHeight * 1.25f);
            break;
        case MapType::World:
            newWidth = static_cast<int>(kMapWidth * 1.75f);
            newHeight = static_cast<int>(kMapHeight * 1.75f);
            break;
        default:
            break;
    }
    if (newWidth != width_ || newHeight != height_) {
        width_ = newWidth;
        height_ = newHeight;
        tiles_.resize(width_ * height_);
    }
    tiles_.assign(width_ * height_, Tile{});

    float centerX = static_cast<float>(width_) * 0.5f;
    float centerY = static_cast<float>(height_) * 0.5f;

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            float nx = static_cast<float>(x) * 0.035f;
            float ny = static_cast<float>(y) * 0.035f;

            float elevation = Fbm2D(nx, ny, seed, 5, 0.5f);

            if (type == MapType::Procedural) {
                float dx = (static_cast<float>(x) - centerX) / centerX;
                float dy = (static_cast<float>(y) - centerY) / centerY;
                float edgeDist = std::sqrt(dx * dx + dy * dy);
                float edgeFalloff = 1.0f - std::clamp(std::pow(edgeDist, 2.5f), 0.0f, 1.0f);
                elevation *= (0.3f + 0.7f * edgeFalloff);
            } else {
                float mask = ContinentMask(x, y, width_, height_, type, seed);
                elevation *= (0.4f + 0.6f * mask);
            }

            if (x < 2 || x >= width_ - 2 || y < 2 || y >= height_ - 2) {
                elevation = 0.1f;
            }

            Tile& tile = GetTile(x, y);
            tile.ownerId = kNeutralPlayerId;
            tile.building = BuildingType::None;
            tile.buildingLevel = 0;
            tile.falloutTimer = 0.0f;
            tile.captureFlashTimer = 0.0f;
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
    // Allow naval invasion: water tiles can be claimed by players with ports
    t.ownerId = owner;
}

void WorldMap::UpdateDefenseBonuses(const std::vector<Building>& buildings) {
    for (auto& t : tiles_) {
        t.hasDefenseBonus = false;
    }
    for (const auto& b : buildings) {
        if (b.type == BuildingType::DefensePost && b.active) {
            for (auto& t : tiles_) {
                // Simple distance check - could be optimized
            }
        }
    }
}

void WorldMap::UpdateBorders() {
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            Tile& t = GetTile(x, y);
            if (t.terrain == TerrainType::Water) {
                t.isBorder = false;
                continue;
            }
            bool isBorder = false;
            for (const auto& n : GetNeighbors4(x, y)) {
                if (GetTile(n.x, n.y).ownerId != t.ownerId) {
                    isBorder = true;
                    break;
                }
            }
            t.isBorder = isBorder;
        }
    }
}

void WorldMap::Tick(float dt) {
    for (auto& t : tiles_) {
        if (t.captureFlashTimer > 0.0f) {
            t.captureFlashTimer = std::max(0.0f, t.captureFlashTimer - dt);
        }
        if (t.falloutTimer > 0.0f) {
            t.falloutTimer = std::max(0.0f, t.falloutTimer - dt);
        }
    }
}

std::vector<TileCoord> WorldMap::GetNeighbors4(int x, int y) const {
    std::vector<TileCoord> nbrs;
    const int dx[] = {0, 0, -1, 1};
    const int dy[] = {-1, 1, 0, 0};
    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (InBounds(nx, ny)) {
            nbrs.push_back({nx, ny});
        }
    }
    return nbrs;
}

std::vector<TileCoord> WorldMap::GetNeighbors8(int x, int y) const {
    std::vector<TileCoord> nbrs;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            int nx = x + dx;
            int ny = y + dy;
            if (InBounds(nx, ny)) {
                nbrs.push_back({nx, ny});
            }
        }
    }
    return nbrs;
}

void WorldMap::SetBuilding(int x, int y, BuildingType type, int level) {
    if (!InBounds(x, y)) {
        return;
    }
    Tile& t = GetTile(x, y);
    if (t.terrain != TerrainType::Water) {
        t.building = type;
        t.buildingLevel = level;
    }
}

void WorldMap::RemoveBuilding(int x, int y) {
    if (!InBounds(x, y)) {
        return;
    }
    Tile& t = GetTile(x, y);
    t.building = BuildingType::None;
    t.buildingLevel = 0;
}

bool WorldMap::HasAdjacentOwner(int x, int y, PlayerId owner) const {
    if (!InBounds(x, y)) {
        return false;
    }
    const auto& nbrs = GetNeighbors4(x, y);
    for (const auto& n : nbrs) {
        if (GetTile(n.x, n.y).ownerId == owner) {
            return true;
        }
    }
    return false;
}

bool WorldMap::IsCoastal(int x, int y) const {
    if (!InBounds(x, y)) {
        return false;
    }
    if (GetTile(x, y).terrain == TerrainType::Water) {
        return false;
    }
    const int dx[] = {0, 0, -1, 1};
    const int dy[] = {-1, 1, 0, 0};
    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (InBounds(nx, ny) && GetTile(nx, ny).terrain == TerrainType::Water) {
            return true;
        }
    }
    return false;
}

} // namespace Openfront