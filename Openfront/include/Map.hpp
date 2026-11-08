#ifndef OPENFRONT_MAP_HPP_
#define OPENFRONT_MAP_HPP_

#include "Common.hpp"
#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace Openfront {

struct Building; // Forward declaration

struct Tile {
    TerrainType terrain{TerrainType::Plains};
    PlayerId ownerId{kNeutralPlayerId};
    BuildingType building{BuildingType::None};
    int buildingLevel{0};
    float falloutTimer{0.0f};
    float captureFlashTimer{0.0f};
    bool hasDefenseBonus{false};
    bool isBorder{false};
};

enum class MapType {
    Procedural,  // Original noise-based generation
    Europe,      // European continent shape
    World,       // World map with multiple continents
    Pangaea,     // Single large landmass
    Archipelago, // Many small islands
};

class WorldMap {
  public:
    WorldMap(int width = kMapWidth, int height = kMapHeight);

    void Generate(uint32_t seed = 42, MapType type = MapType::Procedural);

    static void MapSizeFor(MapType type, int& width, int& height);

    [[nodiscard]] bool InBounds(int x, int y) const {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    [[nodiscard]] const Tile& GetTile(int x, int y) const {
        return tiles_[y * width_ + x];
    }

    [[nodiscard]] Tile& GetTile(int x, int y) {
        return tiles_[y * width_ + x];
    }

    void SetOwner(int x, int y, PlayerId owner);
    void SetBuilding(int x, int y, BuildingType type, int level = 1);
    void RemoveBuilding(int x, int y);

    [[nodiscard]] bool IsCoastal(int x, int y) const;
    [[nodiscard]] bool HasAdjacentOwner(int x, int y, PlayerId owner) const;

    [[nodiscard]] std::vector<TileCoord> GetNeighbors4(int x, int y) const;
    [[nodiscard]] std::vector<TileCoord> GetNeighbors8(int x, int y) const;

    void UpdateDefenseBonuses(const std::vector<Building>& buildings);
    void UpdateBorders();
    void Tick(float dt);

    static constexpr int kDirtyBlock = 64;

    void MarkDirty(int x, int y) {
        if (!InBounds(x, y) || allDirty_) {
            return;
        }
        uint32_t id = static_cast<uint32_t>((y / kDirtyBlock) * GetBlockCols() + (x / kDirtyBlock));
        if (dirtyStamp_[id] == 0) {
            dirtyStamp_[id] = 1;
            dirtyBlocks_.push_back(id);
        }
    }
    void MarkDirtyRect(int x, int y, int w, int h) {
        for (int yy = y; yy < y + h; ++yy) {
            for (int xx = x; xx < x + w; ++xx) {
                MarkDirty(xx, yy);
            }
        }
    }
    void MarkAllDirty() {
        allDirty_ = true;
        dirtyBlocks_.clear();
    }
    [[nodiscard]] bool IsAllDirty() const {
        return allDirty_;
    }
    [[nodiscard]] const std::vector<uint32_t>& GetDirtyBlocks() const {
        return dirtyBlocks_;
    }
    [[nodiscard]] int GetBlockCols() const {
        return (width_ + kDirtyBlock - 1) / kDirtyBlock;
    }
    void ClearDirtyBlocks() {
        allDirty_ = false;
        dirtyBlocks_.clear();
        std::fill(dirtyStamp_.begin(), dirtyStamp_.end(), uint8_t{0});
        allDirty_ = false;
    }

    void StartCaptureFlash(int x, int y) {
        if (!InBounds(x, y)) {
            return;
        }
        GetTile(x, y).captureFlashTimer = 0.5f;
        uint32_t idx = static_cast<uint32_t>(y * width_ + x);
        flashTiles_.push_back(idx);
        MarkDirty(x, y);
    }
    void ApplyFallout(int x, int y, float duration) {
        if (!InBounds(x, y)) {
            return;
        }
        GetTile(x, y).falloutTimer = duration;
        uint32_t idx = static_cast<uint32_t>(y * width_ + x);
        if (falloutTiles_.end() == std::find(falloutTiles_.begin(), falloutTiles_.end(), idx)) {
            falloutTiles_.push_back(idx);
        }
        MarkDirty(x, y);
    }

    [[nodiscard]] const std::vector<TileCoord>& GetBorderTiles() const {
        return borderTiles_;
    }
    [[nodiscard]] const std::vector<TileCoord>& GetCoastalWater() const {
        return coastalWater_;
    }
    void RebuildCoastalWater();

    [[nodiscard]] uint32_t GetOwnerTileCount(PlayerId owner) const {
        auto it = ownerTileCounts_.find(owner);
        return it == ownerTileCounts_.end() ? 0u : it->second;
    }
    void RecountOwnerTiles();

    [[nodiscard]] int GetWidth() const {
        return width_;
    }
    [[nodiscard]] int GetHeight() const {
        return height_;
    }
    [[nodiscard]] int GetTotalTiles() const {
        return width_ * height_;
    }
    [[nodiscard]] int GetLandTilesCount() const {
        return landTilesCount_;
    }

    [[nodiscard]] const std::vector<Tile>& GetTiles() const {
        return tiles_;
    }

  private:
    int width_{kMapWidth};
    int height_{kMapHeight};
    int landTilesCount_{0};
    std::vector<Tile> tiles_;

    std::vector<uint8_t> dirtyStamp_;
    std::vector<uint32_t> dirtyBlocks_;
    bool allDirty_{true};
    void RebuildDirtyGrid() {
        dirtyStamp_.assign((width_ + kDirtyBlock - 1) / kDirtyBlock *
                               ((height_ + kDirtyBlock - 1) / kDirtyBlock),
                           0);
        dirtyBlocks_.clear();
        allDirty_ = true;
    }

    std::vector<uint32_t> flashTiles_;
    std::vector<uint32_t> falloutTiles_;

    std::vector<TileCoord> borderTiles_;
    std::vector<TileCoord> coastalWater_;

    std::unordered_map<PlayerId, uint32_t> ownerTileCounts_;

    void ComputeLandCount();
};

} // namespace Openfront

#endif // OPENFRONT_MAP_HPP_
