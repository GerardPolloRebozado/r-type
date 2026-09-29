#include "SoundPlugin.hpp"

namespace Engine {
    void SoundPlugin::Init(Registry &registry) {
        registry.RegisterComponent<SoundPlugin>();
    }
} // namespace Engine
