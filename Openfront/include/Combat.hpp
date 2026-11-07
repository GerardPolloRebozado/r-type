#ifndef OPENFRONT_COMBAT_HPP_
#define OPENFRONT_COMBAT_HPP_

#include "Common.hpp"
#include <cstdint>
#include <raylib.h>
#include <string>
#include <vector>

namespace Openfront {

class WorldMap;
struct Player;

struct FrontPush {
    uint32_t id{0};
    PlayerId attackerId{kNeutralPlayerId};
    PlayerId targetPlayerId{kNeutralPlayerId};
    int targetX{0};
    int targetY{0};
    double initialTroops{0.0};
    double troops{0.0};
    float rate{12.0f}; // Tiles attempted per second
    float progressAccumulator{0.0f};
    int tilesCaptured{0};
    bool finished{false};
    Vector2 frontCenter{0.0f, 0.0f};
    std::vector<TileCoord> activeBorderTiles;
};

// Aliased for backwards compatibility if needed
using AttackFront = FrontPush;

struct ActiveMissile {
    uint32_t id{0};
    PlayerId ownerId{kNeutralPlayerId};
    BombType type{BombType::AtomBomb};
    Vector2 startPos{0.0f, 0.0f};
    Vector2 currentPos{0.0f, 0.0f};
    Vector2 targetPos{0.0f, 0.0f};
    TileCoord targetTile{0, 0};
    float progress{0.0f};
    float speed{0.15f}; // fraction per second
    float blastRadius{8.0f};
    bool intercepted{false};
    bool exploded{false};
};

struct ExplosionEffect {
    Vector2 pos{0.0f, 0.0f};
    float maxRadius{64.0f};
    float currentRadius{0.0f};
    float duration{1.5f};
    float elapsed{0.0f};
    Color color{ORANGE};
    bool finished{false};
};

struct InterceptionBeam {
    Vector2 samPos{0.0f, 0.0f};
    Vector2 missilePos{0.0f, 0.0f};
    float duration{0.5f};
    float elapsed{0.0f};
    bool finished{false};
};

struct CombatClashParticle {
    Vector2 pos{0.0f, 0.0f};
    Vector2 vel{0.0f, 0.0f};
    Color color{GOLD};
    float lifetime{0.4f};
    float age{0.0f};
};

struct FloatingText {
    Vector2 pos{0.0f, 0.0f};
    std::string text;
    Color color{WHITE};
    float duration{1.2f};
    float age{0.0f};
};

} // namespace Openfront

#endif // OPENFRONT_COMBAT_HPP_
