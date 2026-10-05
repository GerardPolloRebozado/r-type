#include "Plugins/AudioPlugin.hpp"
#include <raylib.h>

namespace Engine {
void AudioPlugin::Init(Registry& registry) {
    if (!registry.HasResource<AssetManager>()) {
        registry.AddResource<AssetManager>();
    }

    InitAudioDevice();
    registry.RegisterComponent<AudioSource>();

    registry.RegisterSystem<AudioSystem>();
}
}
