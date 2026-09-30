#ifndef ENGINE_SYSTEMS_AUDIOSYSTEM_HPP_
#define ENGINE_SYSTEMS_AUDIOSYSTEM_HPP_

#include "ECS/System.hpp"
#include "ECS/Registry.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/AudioSource.hpp"
#include <vector>
#include <ranges>

namespace Engine {
    /**
     * @class AudioSystem
     * @brief A pure ECS System that processes AudioSource data components and maintains audio streams.
     */
    class AudioSystem : public System {
        // names of currently playing sounds
        std::vector<Sound> m_ActiveSounds;

    public:
        void Update(Registry &registry, float dt) override {
            if (!registry.HasResource<AssetManager>()) return;
            auto &assetManager = registry.GetResource<AssetManager>();

            // update each sound stream playing
            for (const auto &music: assetManager.GetAllMusic() | std::views::values) {
                if (music->IsPlaying()) {
                    music->Update();
                }
            }

            // unload them after playing them
            std::erase_if(m_ActiveSounds, [](const Engine::Sound &s) {
                return !s.IsPlaying();
            });

            for (auto entity: m_Entities) {
                auto &audio = registry.GetComponent<AudioSource>(entity);

                if (audio.shouldPlay) {
                    if (audio.isMusic) {
                        auto &music = assetManager.GetMusic(audio.assetId);
                        music.Play();
                    } else {
                        Sound alias = assetManager.GetSound(audio.assetId);
                        alias.Play();
                        m_ActiveSounds.push_back(std::move(alias));
                    }

                    audio.shouldPlay = false;
                    audio.isPlaying = true;
                }

                if (audio.shouldStop) {
                    if (audio.isMusic) {
                        assetManager.GetMusic(audio.assetId).Stop();
                    }

                    audio.shouldStop = false;
                    audio.isPlaying = false;
                }
            }
        }
    };
} // namespace Engine

#endif // ENGINE_SYSTEMS_AUDIOSYSTEM_HPP_
