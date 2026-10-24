#ifndef ENGINE_SYSTEMS_AUDIOSYSTEM_HPP_
#define ENGINE_SYSTEMS_AUDIOSYSTEM_HPP_

#include "Assets/Sound.hpp"
#include "ECS/System.hpp"
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace Engine {
class AssetManager;
class Music;
struct AudioSource;

/**
 * @class AudioSystem
 * @brief A pure ECS System that processes AudioSource data components and maintains audio streams.
 */
class AudioSystem : public System {
    struct ActiveSound {
        Entity owner;
        Sound sound;
    };

    std::vector<ActiveSound> m_ActiveSounds;
    std::unordered_set<std::string> m_WarnedMissing;

    void WarnMissing(const std::string& assetId, const std::string& reason);

    std::shared_ptr<Music> GetOrLoadMusic(Registry& registry, AssetManager& assetManager,
                                          const std::string& assetId, bool loop);

    std::shared_ptr<Sound> GetOrLoadSound(Registry& registry, AssetManager& assetManager,
                                          const std::string& assetId, bool loop);

    void UpdateAssetAudio(AssetManager& assetManager);

    void UpdateSoundInstances();

    void StopMusicStream(AssetManager& assetManager, const std::string& assetId);

    void StopOwnedSoundInstances(Entity owner);

    void StopSource(Entity entity, AudioSource& source, AssetManager& assetManager);

    void StartMusicSource(Registry& registry, AudioSource& source, AssetManager& assetManager);

    void StartSoundSource(Entity entity, Registry& registry, AudioSource& source,
                          AssetManager& assetManager);

    void StartSource(Entity entity, Registry& registry, AudioSource& source,
                     AssetManager& assetManager);

    void RefreshMusicSource(AudioSource& source, AssetManager& assetManager);

    void RefreshSoundSource(Entity entity, AudioSource& source);

    void RefreshSourceState(Entity entity, AudioSource& source, AssetManager& assetManager);

    void ProcessSource(Entity entity, Registry& registry, AssetManager& assetManager);

  public:
    void Update(Registry& registry, float dt) override;
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_AUDIOSYSTEM_HPP_
