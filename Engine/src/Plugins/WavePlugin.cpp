#include "Plugins/WavePlugin.hpp"

namespace Engine {

void WavePlugin::Init(Registry& registry) {
    registry.RegisterSystem<WaveSpawnerSystem>();
}

} // namespace Engine
