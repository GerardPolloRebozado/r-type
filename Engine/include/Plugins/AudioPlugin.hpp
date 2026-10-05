#ifndef ENGINE_PLUGINS_AUDIOPLUGIN_HPP_
#define ENGINE_PLUGINS_AUDIOPLUGIN_HPP_

#include "Assets/AssetManager.hpp"
#include "Components/AudioSource.hpp"
#include "Plugins/IPlugin.hpp"
#include "Systems/AudioSystem.hpp"

namespace Engine {
/**
 * @class AudioPlugin
 * @brief Bundles all audio-related resources, components, and systems for easy engine integration.
 */
class AudioPlugin : public IPlugin {
  public:
    void Init(Registry& registry) override {
        if (!registry.HasResource<AssetManager>()) {
            registry.AddResource<AssetManager>();
        }

        InitAudioDevice();
        registry.RegisterComponent<AudioSource>();

        registry.RegisterSystem<AudioSystem>();
    }
};
} // namespace Engine

#endif // ENGINE_PLUGINS_AUDIOPLUGIN_HPP_
