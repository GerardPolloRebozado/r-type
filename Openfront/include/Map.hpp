#ifndef OPENFRONT_MAP_HPP_
#define OPENFRONT_MAP_HPP_

#include "Common.hpp"
#include <cstdint>
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

    void ComputeLandCount();
};

} // namespace Openfront

#endif // OPENFRONT_MAP_HPP_
