#ifndef OPENFRONT_PLAYER_HPP_
#define OPENFRONT_PLAYER_HPP_

#include "Common.hpp"
#include "NetworkProtocol.hpp"
#include <string>
#include <vector>

namespace Openfront {

class Simulation; // Forward declaration

enum class AIPersonality {
    Balanced,
    Aggressive,
    Defensive,
    Economic,
};

struct Player {
    PlayerId id{kNeutralPlayerId};
    std::string name{"Neutral"};
    FactionType type{FactionType::Bot};
    AIPersonality personality{AIPersonality::Balanced};
    Color color{GRAY};
    bool isAlive{true};

    int tileCount{0};
    double troops{50000.0};
    double maxTroops{100000.0};
    double gold{10000.0};
    float attackRatio{0.20f}; // Default 20%

    // Building counts / levels
    int cityCount{0};
    int cityLevelsSum{0};
    int portCount{0};
    int factoryCount{0};
    int defensePostCount{0};
    int samCount{0};
    int siloCount{0};

    // AI timers and memory
    float aiActionTimer{0.0f};
    float aiBuildTimer{0.0f};
    float aiNukeTimer{0.0f};

    void CalculateMaxTroops();
    void TickEconomy(float dt);
    [[nodiscard]] double GetTroopDensity() const;
    [[nodiscard]] double GetShield() const;

    void UpdateAI(Simulation& sim, std::vector<ClientIntent>& outIntents, float dt);
};

} // namespace Openfront

#endif // OPENFRONT_PLAYER_HPP_
