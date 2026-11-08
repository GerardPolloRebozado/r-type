#include "Map.hpp"
#include "Building.hpp"
#include "GeoMasks.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <thread>

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

float ContinentMask(float u, float v, MapType type, uint32_t seed) {
    auto distFrom = [&](float px, float py, float rx, float ry) -> float {
        float dx = (u - px) / rx;
        float dy = (v - py) / ry;
        return std::sqrt(dx * dx + dy * dy);
    };
    auto addBlob = [&](float& mask, float px, float py, float rx, float ry, float powExp) {
        float d = distFrom(px, py, rx, ry);
        mask = std::max(mask, 1.0f - std::clamp(std::pow(d, powExp), 0.0f, 1.0f));
    };

    switch (type) {
        case MapType::Procedural: {
            float dx = (u - 0.5f) / 0.58f;
            float dy = (v - 0.5f) / 0.58f;
            float dist = std::sqrt(dx * dx + dy * dy);
            float falloff = 1.0f - std::clamp(std::pow(dist, 2.6f), 0.0f, 1.0f);
            float patch = Fbm2D(u * 4.0f, v * 2.5f, seed + 777, 3, 0.55f);
            return std::max(falloff, Smoothstep(0.73f, 0.79f, patch));
        }
        case MapType::Pangaea: {
            float dx = (u - 0.5f) / 0.45f;
            float dy = (v - 0.5f) / 0.42f;
            float dist = std::sqrt(dx * dx + dy * dy);
            float mask = 1.0f - std::clamp(std::pow(dist, 1.4f), 0.0f, 1.0f);
            float bay = Fbm2D(u * 6.0f, v * 6.0f, seed + 200, 4, 0.5f);
            mask *= (0.72f + 0.28f * bay);
            return mask;
        }
        case MapType::Archipelago: {
            float noise = Fbm2D(u * 3.5f, v * 2.2f, seed + 500, 3, 0.6f);
            float isles = 0.0f;
            addBlob(isles, 0.30f, 0.45f, 0.10f, 0.07f, 1.6f);
            addBlob(isles, 0.70f, 0.35f, 0.09f, 0.06f, 1.6f);
            addBlob(isles, 0.45f, 0.72f, 0.13f, 0.09f, 1.6f);
            addBlob(isles, 0.68f, 0.68f, 0.08f, 0.06f, 1.6f);
            addBlob(isles, 0.15f, 0.65f, 0.08f, 0.06f, 1.6f);
            addBlob(isles, 0.85f, 0.40f, 0.07f, 0.05f, 1.6f);
            isles *= (0.85f + 0.35f * noise);
            float mask = Smoothstep(0.62f, 0.68f, noise);
            return std::max(mask, isles);
        }
        default:
            return 1.0f;
    }
}

const GeoMask::Mask* GeoMaskFor(MapType type) {
    switch (type) {
        case MapType::Europe:
            return &GeoMask::EuropeMask;
        case MapType::World:
            return &GeoMask::WorldMask;
        default:
            return nullptr;
    }
}

std::vector<float> DecodeGeoMask(const GeoMask::Mask& m) {
    std::vector<float> grid(static_cast<size_t>(m.gridW) * static_cast<size_t>(m.gridH));
    const char* p = m.rle;
    size_t i = 0;
    while (*p && i < grid.size()) {
        float value = ((*p == 'L') || (*p == 'l')) ? 1.0f : 0.0f;
        ++p;
        int count = 0;
        while (*p >= '0' && *p <= '9') {
            count = count * 10 + (*p - '0');
            ++p;
        }
        for (int k = 0; k < count && i < grid.size(); ++k) {
            grid[i++] = value;
        }
    }
    return grid;
}

float SampleGeoGrid(const float* grid, int gw, int gh, float u, float v) {
    float gx = std::clamp(u * gw - 0.5f, 0.0f, gw - 1.0f);
    float gy = std::clamp(v * gh - 0.5f, 0.0f, gh - 1.0f);
    int x0 = std::clamp(static_cast<int>(gx), 0, gw - 1);
    int y0 = std::clamp(static_cast<int>(gy), 0, gh - 1);
    int x1 = std::min(x0 + 1, gw - 1);
    int y1 = std::min(y0 + 1, gh - 1);
    float tx = gx - x0;
    float ty = gy - y0;
    tx = tx * tx * (3.0f - 2.0f * tx);
    ty = ty * ty * (3.0f - 2.0f * ty);
    float v00 = grid[y0 * gw + x0];
    float v10 = grid[y0 * gw + x1];
    float v01 = grid[y1 * gw + x0];
    float v11 = grid[y1 * gw + x1];
    float a = v00 + tx * (v10 - v00);
    float b = v01 + tx * (v11 - v01);
    return a + ty * (b - a);
}

} // namespace

WorldMap::WorldMap(int width, int height) : width_(width), height_(height), tiles_(width * height) {
    RebuildDirtyGrid();
    ComputeLandCount();
}

void WorldMap::RebuildCoastalWater() {
    coastalWater_.clear();
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            if (GetTile(x, y).terrain != TerrainType::Water) {
                continue;
            }
            static const int dx[] = {0, 0, -1, 1};
            static const int dy[] = {-1, 1, 0, 0};
            bool coastal = false;
            for (int i = 0; i < 4 && !coastal; ++i) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                if (InBounds(nx, ny) && GetTile(nx, ny).terrain != TerrainType::Water) {
                    coastal = true;
                }
            }
            if (coastal) {
                coastalWater_.push_back({x, y});
            }
        }
    }
}

void WorldMap::MapSizeFor(MapType type, int& width, int& height) {
    switch (type) {
        case MapType::Procedural:
            width = 900;
            height = 562;
            break;
        case MapType::Europe:
            width = 1100;
            height = 875;
            break;
        case MapType::Pangaea:
        case MapType::Archipelago:
            width = 1100;
            height = 688;
            break;
        case MapType::World:
            width = 1600;
            height = 930;
            break;
        default:
            width = 900;
            height = 562;
            break;
    }
}

void WorldMap::Generate(uint32_t seed, MapType type) {
    int newWidth = 0;
    int newHeight = 0;
    MapSizeFor(type, newWidth, newHeight);
    if (newWidth != width_ || newHeight != height_) {
        width_ = newWidth;
        height_ = newHeight;
        tiles_.resize(width_ * height_);
        RebuildDirtyGrid();
    }
    tiles_.assign(width_ * height_, Tile{});

    const GeoMask::Mask* geoMask = GeoMaskFor(type);
    std::vector<float> geoGrid;
    int geoW = 0;
    int geoH = 0;
    if (geoMask) {
        geoGrid = DecodeGeoMask(*geoMask);
        geoW = geoMask->gridW;
        geoH = geoMask->gridH;
    }
    const float* geoData = geoGrid.data();

    unsigned threadCount = std::max(1u, std::thread::hardware_concurrency());
    int rowsPerThread =
        (height_ + static_cast<int>(threadCount) - 1) / static_cast<int>(threadCount);
    auto generateRows = [this, type, seed, geoData, geoW, geoH](int y0, int y1) {
        int width = width_;
        for (int y = y0; y < y1 && y < height_; ++y) {
            for (int x = 0; x < width; ++x) {
                float u = static_cast<float>(x) / static_cast<float>(width_);
                float v = static_cast<float>(y) / static_cast<float>(height_);

                float warp = (geoData != nullptr) ? 0.03f : 0.07f;
                float wx = Fbm2D(u * 3.0f, v * 2.0f, seed + 941, 3, 0.55f) - 0.5f;
                float wy = Fbm2D(u * 3.0f, v * 2.0f, seed + 977, 3, 0.55f) - 0.5f;
                float uu = std::clamp(u + wx * warp, 0.0f, 1.0f);
                float vv = std::clamp(v + wy * warp, 0.0f, 1.0f);

                float structure = Fbm2D(uu * 5.6f, vv * 3.5f, seed, 4, 0.5f);
                float mid = Fbm2D(uu * 16.0f, vv * 10.0f, seed + 321, 2, 0.5f);
                float coast = Fbm2D(static_cast<float>(x) * 0.11f, static_cast<float>(y) * 0.11f,
                                    seed + 61, 2, 0.5f);

                float elevation = 0.72f * structure + 0.28f * mid;
                float mask;
                if (geoData != nullptr) {
                    mask = SampleGeoGrid(geoData, geoW, geoH, uu, vv);
                    float inland = Smoothstep(0.85f, 1.0f, mask);
                    elevation = std::max(elevation, inland * 0.36f);
                    elevation *= (0.22f + 0.78f * mask);
                } else if (type == MapType::Procedural) {
                    mask = ContinentMask(uu, vv, type, seed);
                    elevation *= (0.30f + 0.70f * mask);
                } else {
                    mask = ContinentMask(uu, vv, type, seed);
                    elevation *= (0.48f + 0.52f * mask);
                }
                elevation *= (0.90f + 0.10f * coast);

                float ridge = Fbm2D(u * 26.0f, v * 16.0f, seed + 444, 3, 0.55f);
                ridge = 1.0f - std::fabs(2.0f * ridge - 1.0f);

                if (x < 2 || x >= width_ - 2 || y < 2 || y >= height_ - 2) {
                    elevation = 0.0f;
                }

                Tile& tile = GetTile(x, y);
                tile.ownerId = kNeutralPlayerId;
                tile.building = BuildingType::None;
                tile.buildingLevel = 0;
                tile.falloutTimer = 0.0f;
                tile.captureFlashTimer = 0.0f;
                tile.hasDefenseBonus = false;

                if (elevation < 0.30f) {
                    tile.terrain = TerrainType::Water;
                } else {
                    float marine = Smoothstep(0.30f, 0.36f, elevation);
                    float ranges = ridge * marine;
                    if (ranges > 0.87f) {
                        tile.terrain = TerrainType::Mountains;
                    } else if (ranges > 0.78f || elevation > 0.52f) {
                        tile.terrain = TerrainType::Highlands;
                    } else {
                        tile.terrain = TerrainType::Plains;
                    }
                }
            }
        }
    };
    {
        std::vector<std::thread> pool;
        pool.reserve(threadCount);
        for (unsigned i = 0; i < threadCount; ++i) {
            pool.emplace_back(generateRows, i * rowsPerThread,
                              std::min(height_, static_cast<int>((i + 1) * rowsPerThread)));
        }
        for (auto& th : pool) {
            th.join();
        }
    }

    ComputeLandCount();
    UpdateBorders();

    RebuildCoastalWater();

    MarkAllDirty();
    flashTiles_.clear();
    falloutTiles_.clear();
    RecountOwnerTiles();
    borderTiles_.clear();
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
    if (t.terrain == TerrainType::Water && owner != kNeutralPlayerId) {
        return;
    }
    if (t.ownerId != owner) {
        if (t.ownerId != kNeutralPlayerId) {
            auto it = ownerTileCounts_.find(t.ownerId);
            if (it != ownerTileCounts_.end() && it->second > 0) {
                it->second--;
            }
        }
        if (owner != kNeutralPlayerId) {
            ownerTileCounts_[owner]++;
        }
        t.ownerId = owner;
        MarkDirty(x, y);
    }
}

void WorldMap::RecountOwnerTiles() {
    ownerTileCounts_.clear();
    for (const Tile& t : tiles_) {
        if (t.ownerId != kNeutralPlayerId) {
            ownerTileCounts_[t.ownerId]++;
        }
    }
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
    borderTiles_.clear();
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            Tile& t = tiles_[static_cast<size_t>(y) * width_ + x];
            if (t.terrain == TerrainType::Water) {
                t.isBorder = false;
                continue;
            }
            PlayerId owner = t.ownerId;
            bool isBorder =
                (x > 0 && tiles_[static_cast<size_t>(y) * width_ + x - 1].ownerId != owner) ||
                (x < width_ - 1 &&
                 tiles_[static_cast<size_t>(y) * width_ + x + 1].ownerId != owner) ||
                (y > 0 && tiles_[static_cast<size_t>(y - 1) * width_ + x].ownerId != owner) ||
                (y < height_ - 1 &&
                 tiles_[static_cast<size_t>(y + 1) * width_ + x].ownerId != owner);
            if (t.isBorder != isBorder) {
                MarkDirty(x, y);
            }
            t.isBorder = isBorder;
            if (isBorder) {
                borderTiles_.push_back({x, y});
            }
        }
    }
}

void WorldMap::Tick(float dt) {
    for (size_t i = 0; i < flashTiles_.size();) {
        uint32_t idx = flashTiles_[i];
        Tile& t = tiles_[idx];
        if (t.captureFlashTimer > 0.0f) {
            t.captureFlashTimer = std::max(0.0f, t.captureFlashTimer - dt);
            if (t.captureFlashTimer == 0.0f) {
                flashTiles_[i] = flashTiles_.back();
                flashTiles_.pop_back();
            } else {
                ++i;
            }
            MarkDirty(static_cast<int>(idx % width_), static_cast<int>(idx / width_));
        } else {
            flashTiles_[i] = flashTiles_.back();
            flashTiles_.pop_back();
        }
    }
    for (size_t i = 0; i < falloutTiles_.size();) {
        uint32_t idx = falloutTiles_[i];
        Tile& t = tiles_[idx];
        if (t.falloutTimer > 0.0f) {
            t.falloutTimer = std::max(0.0f, t.falloutTimer - dt);
            if (t.falloutTimer == 0.0f) {
                falloutTiles_[i] = falloutTiles_.back();
                falloutTiles_.pop_back();
                MarkDirty(static_cast<int>(idx % width_), static_cast<int>(idx / width_));
            } else {
                ++i;
            }
        } else {
            falloutTiles_[i] = falloutTiles_.back();
            falloutTiles_.pop_back();
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
        MarkDirty(x, y);
    }
}

void WorldMap::RemoveBuilding(int x, int y) {
    if (!InBounds(x, y)) {
        return;
    }
    Tile& t = GetTile(x, y);
    t.building = BuildingType::None;
    t.buildingLevel = 0;
    MarkDirty(x, y);
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
