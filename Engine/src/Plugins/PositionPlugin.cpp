#include "Plugins/PositionPlugin.hpp"

namespace Engine {
void PositionPlugin::Init(Registry& registry) {
    registry.RegisterComponent<Position>();

    registry.RegisterSystem<PositionSystem>();
}
}
