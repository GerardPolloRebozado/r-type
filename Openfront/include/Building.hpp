#ifndef OPENFRONT_BUILDING_HPP_
#define OPENFRONT_BUILDING_HPP_

#include "Common.hpp"

namespace Openfront {

struct Building {
    BuildingType type{BuildingType::None};
    PlayerId ownerId{kNeutralPlayerId};
    int x{0};
    int y{0};
    int level{1};
    float timer{0.0f};  // Timer for periodic events (trade ships/trains)
    float range{30.0f}; // Effective radius for DefensePost / SAM
    bool active{true};

    static float DefaultRange(BuildingType t, int lvl) {
        if (t == BuildingType::DefensePost) {
            return 15.0f;
        }
        if (t == BuildingType::SamLauncher) {
            return 15.0f + lvl * 5.0f;
        }
        return 0.0f;
    }
};

} // namespace Openfront

#endif // OPENFRONT_BUILDING_HPP_
