#ifndef OPENFRONT_COMPONENTS_HPP_
#define OPENFRONT_COMPONENTS_HPP_

#include "Common.hpp"
#include <cstdint>

namespace Openfront {

/**
 * @struct TacticalUnitComponent
 * @brief ECS component representing a tactical infrastructure building in Openfront.
 */
struct TacticalUnitComponent {
    BuildingType type{BuildingType::None};
    PlayerId ownerId{kNeutralPlayerId};
    int level{1};
    int tileX{0};
    int tileY{0};
    float timer{0.0f};
    float range{0.0f};
    bool active{true};
};

/**
 * @struct WarheadComponent
 * @brief ECS component representing an in-flight ballistic nuclear warhead.
 */
struct WarheadComponent {
    uint32_t missileId{0};
    PlayerId ownerId{kNeutralPlayerId};
    BombType type{BombType::AtomBomb};
    float progress{0.0f};
    float currentX{0.0f};
    float currentY{0.0f};
    float targetX{0.0f};
    float targetY{0.0f};
};

/**
 * @struct FrontLineComponent
 * @brief ECS component representing an active border expansion push front.
 */
struct FrontLineComponent {
    uint32_t frontId{0};
    PlayerId attackerId{kNeutralPlayerId};
    PlayerId targetId{kNeutralPlayerId};
    double troops{0.0};
    float centerX{0.0f};
    float centerY{0.0f};
};

} // namespace Openfront

#endif // OPENFRONT_COMPONENTS_HPP_
