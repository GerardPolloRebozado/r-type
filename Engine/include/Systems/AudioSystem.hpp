#ifndef ENGINE_SYSTEMS_AUDIOSYSTEM_HPP_
#define ENGINE_SYSTEMS_AUDIOSYSTEM_HPP_

#include "Assets/AssetManager.hpp"
#include "Components/AudioSource.hpp"
#include "ECS/Registry.hpp"
#include "ECS/System.hpp"
#include <ranges>
#include <vector>

namespace Engine {
/**
 * @class AudioSystem
 * @brief A pure ECS System that processes AudioSource data components and maintains audio streams.
 */
class AudioSystem : public System {
    // names of currently playing sounds
    std::vector<Sound> m_ActiveSounds;

  public:
    /**
     * @brief Stops every sound and music track when the scene changes.
     *
     * Audio belongs to the scene that started it. The new scene starts its
     * own music through its AudioSource components. Loaded audio assets stay
     * in the AssetManager, so replaying them doesn't reload files.
     */
    void OnSceneUnloaded(Registry& registry) override {
        for (auto& sound : m_ActiveSounds) {
            sound.Stop();
        }
        m_ActiveSounds.clear();
        if (!registry.HasResource<AssetManager>()) {
            return;
        }
        auto& assetManager = registry.GetResource<AssetManager>();
        for (const auto& music : assetManager.GetAllMusic() | std::views::values) {
            if (music && music->IsPlaying()) {
                music->Stop();
            }
        }
        for (const auto& sound : assetManager.GetAllSounds() | std::views::values) {
            if (sound && sound->IsPlaying()) {
                sound->Stop();
            }
        }
    }

    void Update(Registry& registry, float dt) override {
        if (!registry.HasResource<AssetManager>()) {
            return;
        }
        auto& assetManager = registry.GetResource<AssetManager>();

        // update each sound stream playing
        for (const auto& music : assetManager.GetAllMusic() | std::views::values) {
            if (music->IsPlaying()) {
                music->Update();
            }
        }

        // unload them after playing them
        std::erase_if(m_ActiveSounds, [](const Engine::Sound& s) { return !s.IsPlaying(); });

        for (auto entity : m_Entities) {
            auto& audio = registry.GetComponent<AudioSource>(entity);

            if (audio.shouldPlay) {
                if (audio.isMusic) {
                    auto music = assetManager.GetMusic(audio.assetId);
                    music->Play();
                } else {
                    auto alias = assetManager.GetSound(audio.assetId);
                    alias->Play();
                    m_ActiveSounds.push_back(*alias);
                }

                audio.shouldPlay = false;
                audio.isPlaying = true;
            }

            if (audio.shouldStop) {
                if (audio.isMusic) {
                    assetManager.GetMusic(audio.assetId)->Stop();
                }

                audio.shouldStop = false;
                audio.isPlaying = false;
            }
        }
    }
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_AUDIOSYSTEM_HPP_
