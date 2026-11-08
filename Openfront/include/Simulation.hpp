#ifndef OPENFRONT_SIMULATION_HPP_
#define OPENFRONT_SIMULATION_HPP_

#include "Building.hpp"
#include "Combat.hpp"
#include "Common.hpp"
#include "Map.hpp"
#include "NetworkProtocol.hpp"
#include "OpenfrontComponents.hpp"
#include "Player.hpp"
#include <memory>
#include <vector>

namespace Engine {
class Registry;
}

namespace Openfront {

class Simulation {
  public:
    Simulation();

    void Init(int numNations = 4, int numBots = 4, uint32_t seed = 1337,
              MapType mapType = MapType::Procedural);
    void Tick(float dt);               // 10 Hz simulation logic
    void UpdateVisuals(float frameDt); // 60 FPS visual interpolation

    // Multiplayer Turn & Intent Execution
    void ApplyIntent(const ClientIntent& intent);
    void ExecuteTurn(const std::vector<ClientIntent>& intents, float dt = kSimulationTickDt);
    void CollectAIIntents(std::vector<ClientIntent>& outIntents, float dt = kSimulationTickDt);
    bool AssignPlayerSlot(PlayerId id, const std::string& name,
                          FactionType type = FactionType::Human);
    void CancelAllFrontsForPlayer(PlayerId playerId);

    // Player Commands
    bool LaunchAttack(PlayerId playerId, int targetX, int targetY, float ratio = -1.0f);
    bool LaunchAttack(PlayerId playerId, int startX, int startY, int targetX, int targetY,
                      float ratio = -1.0f);
    bool ReinforceFront(PlayerId playerId, uint32_t frontId);
    bool CancelFront(PlayerId playerId, uint32_t frontId);
    bool Build(PlayerId playerId, int x, int y, BuildingType type);
    bool LaunchNuke(PlayerId playerId, int siloX, int siloY, int targetX, int targetY,
                    BombType type);

    void SpawnFloatingText(Vector2 pos, const std::string& text, Color color = WHITE);
    void SpawnClashParticles(Vector2 pos, Color color = GOLD, int count = 6);

    // Queries
    [[nodiscard]] Player* GetPlayer(PlayerId id);
    [[nodiscard]] const Player* GetPlayer(PlayerId id) const;
    [[nodiscard]] Building* GetBuildingAt(int x, int y);
    [[nodiscard]] const Building* GetBuildingAt(int x, int y) const;

    [[nodiscard]] WorldMap& GetMap() {
        return map_;
    }
    [[nodiscard]] const WorldMap& GetMap() const {
        return map_;
    }
    [[nodiscard]] std::vector<Player>& GetPlayers() {
        return players_;
    }
    [[nodiscard]] const std::vector<Player>& GetPlayers() const {
        return players_;
    }
    [[nodiscard]] std::vector<Building>& GetBuildings() {
        return buildings_;
    }
    [[nodiscard]] const std::vector<Building>& GetBuildings() const {
        return buildings_;
    }
    [[nodiscard]] const std::vector<FrontPush>& GetAttackFronts() const {
        return attackFronts_;
    }
    [[nodiscard]] const std::vector<ActiveMissile>& GetMissiles() const {
        return missiles_;
    }
    [[nodiscard]] const std::vector<ExplosionEffect>& GetExplosions() const {
        return explosions_;
    }
    [[nodiscard]] const std::vector<InterceptionBeam>& GetBeams() const {
        return beams_;
    }
    [[nodiscard]] const std::vector<CombatClashParticle>& GetClashParticles() const {
        return clashParticles_;
    }
    [[nodiscard]] const std::vector<FloatingText>& GetFloatingTexts() const {
        return floatingTexts_;
    }

    [[nodiscard]] float GetMatchTime() const {
        return matchTime_;
    }
    [[nodiscard]] float GetMatchDuration() const {
        return matchDuration_;
    }
    [[nodiscard]] bool IsGameOver() const {
        return gameOver_;
    }
    [[nodiscard]] PlayerId GetWinnerId() const {
        return winnerId_;
    }
    [[nodiscard]] uint32_t GetCurrentTurn() const {
        return currentTurn_;
    }
    void SetCurrentTurn(uint32_t turn) {
        currentTurn_ = turn;
    }
    void RecalculateTerritoryAndBuildings();

    // ECS Integration
    void SetEcsRegistry(Engine::Registry* registry) {
        ecsRegistry_ = registry;
    }
    void SyncToEcs();

    // Helpers
    std::vector<TileCoord> FindPath(int startX, int startY, int targetX, int targetY,
                                    bool allowWater = false);

  private:
    WorldMap map_;
    std::vector<Player> players_;
    std::vector<Building> buildings_;
    std::vector<AttackFront> attackFronts_;
    std::vector<ActiveMissile> missiles_;
    std::vector<ExplosionEffect> explosions_;
    std::vector<InterceptionBeam> beams_;
    std::vector<CombatClashParticle> clashParticles_;
    std::vector<FloatingText> floatingTexts_;

    float matchTime_{0.0f};
    float matchDuration_{900.0f}; // 15 minutes
    uint32_t currentTurn_{0};
    bool gameOver_{false};
    PlayerId winnerId_{kNeutralPlayerId};

    uint32_t nextFrontId_{1};
    uint32_t nextMissileId_{1};

    Engine::Registry* ecsRegistry_{nullptr};
    std::vector<size_t> ecsEntities_;

    void SpawnPlayerBase(PlayerId id, int centerX, int centerY, int radius = 3);
    void TickCombatFronts(float dt);
    void TickMissiles(float dt);
    void ResolveTileConquest(AttackFront& front, int tx, int ty);
    void CheckWinConditions();
};

} // namespace Openfront

#endif // OPENFRONT_SIMULATION_HPP_
