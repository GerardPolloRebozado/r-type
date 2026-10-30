#include "Systems/AudioSystem.hpp"
#include "Assets/AssetManager.hpp"
#include "Assets/ProjectPaths.hpp"
#include "Components/AudioSource.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include <exception>
#include <ranges>
#include <utility>

namespace Engine {

void AudioSystem::WarnMissing(const std::string& assetId, const std::string& reason) {
    if (m_WarnedMissing.insert(assetId).second) {
        log_user_error("[AudioSystem] Cannot play '" + assetId + "': " + reason);
    }
}

std::shared_ptr<Music> AudioSystem::GetOrLoadMusic(Registry& registry, AssetManager& assetManager,
                                                   const std::string& assetId, bool loop) {
    try {
        return assetManager.GetMusic(assetId);
    } catch (...) {
    }

    try {
        assetManager.LoadMusic(assetId, ResolveAssetPath(registry, assetId).string(), loop);
        return assetManager.GetMusic(assetId);
    } catch (const std::exception& error) {
        WarnMissing(assetId, error.what());
    } catch (...) {
        WarnMissing(assetId, "unknown audio error");
    }
    return nullptr;
}

std::shared_ptr<Sound> AudioSystem::GetOrLoadSound(Registry& registry, AssetManager& assetManager,
                                                   const std::string& assetId, bool loop) {
    try {
        return assetManager.GetSound(assetId);
    } catch (...) {
    }

    try {
        assetManager.LoadSound(assetId, ResolveAssetPath(registry, assetId).string(), loop);
        return assetManager.GetSound(assetId);
    } catch (const std::exception& error) {
        WarnMissing(assetId, error.what());
    } catch (...) {
        WarnMissing(assetId, "unknown audio error");
    }
    return nullptr;
}

void AudioSystem::UpdateAssetAudio(AssetManager& assetManager) {
    for (const auto& music : assetManager.GetAllMusic() | std::views::values) {
        if (music && music->IsPlaying()) {
            music->Update();
        }
    }

    for (const auto& sound : assetManager.GetAllSounds() | std::views::values) {
        if (sound) {
            sound->Update();
        }
    }
}

void AudioSystem::UpdateSoundInstances() {
    for (auto& active : m_ActiveSounds) {
        active.sound.Update();
    }
    std::erase_if(m_ActiveSounds, [](const ActiveSound& active) {
        return !active.sound.IsLooping() && !active.sound.IsPlaying();
    });
}

void AudioSystem::StopMusicStream(AssetManager& assetManager, const std::string& assetId) {
    try {
        assetManager.GetMusic(assetId)->Stop();
    } catch (...) {
    }
}

void AudioSystem::StopOwnedSoundInstances(Entity owner) {
    for (auto& active : m_ActiveSounds) {
        if (active.owner == owner) {
            active.sound.Stop();
        }
    }
    std::erase_if(m_ActiveSounds,
                  [owner](const ActiveSound& active) { return active.owner == owner; });
}

void AudioSystem::StopSource(Entity entity, AudioSource& source, AssetManager& assetManager) {
    StopMusicStream(assetManager, source.assetId);
    StopOwnedSoundInstances(entity);
    source.shouldStop = false;
    source.isPlaying = false;
}

void AudioSystem::StartMusicSource(Registry& registry, AudioSource& source,
                                   AssetManager& assetManager) {
    auto music = GetOrLoadMusic(registry, assetManager, source.assetId, source.loop);
    if (!music) {
        source.isPlaying = false;
        return;
    }

    music->SetLooping(source.loop);
    music->SetVolume(source.volume);
    music->Play();
    source.isPlaying = true;
}

void AudioSystem::StartSoundSource(Entity entity, Registry& registry, AudioSource& source,
                                   AssetManager& assetManager) {
    auto asset = GetOrLoadSound(registry, assetManager, source.assetId, source.loop);
    if (!asset) {
        source.isPlaying = false;
        return;
    }

    Sound instance(*asset);
    instance.SetLooping(source.loop);
    instance.SetVolume(source.volume);
    instance.Play();
    m_ActiveSounds.push_back(ActiveSound{entity, std::move(instance)});
    source.isPlaying = true;
}

void AudioSystem::StartSource(Entity entity, Registry& registry, AudioSource& source,
                              AssetManager& assetManager) {
    source.shouldPlay = false;

    if (source.isMusic) {
        StartMusicSource(registry, source, assetManager);
    } else {
        StartSoundSource(entity, registry, source, assetManager);
    }
}

void AudioSystem::RefreshMusicSource(AudioSource& source, AssetManager& assetManager) {
    try {
        auto music = assetManager.GetMusic(source.assetId);
        source.isPlaying = music->IsPlaying();
        if (source.isPlaying) {
            music->SetLooping(source.loop);
            music->SetVolume(source.volume);
        }
    } catch (...) {
        source.isPlaying = false;
    }
}

void AudioSystem::RefreshSoundSource(Entity entity, AudioSource& source) {
    source.isPlaying = false;

    for (auto& active : m_ActiveSounds) {
        if (active.owner != entity) {
            continue;
        }

        active.sound.SetLooping(source.loop);
        active.sound.SetVolume(source.volume);
        if (active.sound.IsPlaying()) {
            source.isPlaying = true;
        }
    }
}

void AudioSystem::RefreshSourceState(Entity entity, AudioSource& source,
                                     AssetManager& assetManager) {
    if (source.shouldPlay || !source.isPlaying) {
        return;
    }

    if (source.isMusic) {
        RefreshMusicSource(source, assetManager);
    } else {
        RefreshSoundSource(entity, source);
    }
}

void AudioSystem::ProcessSource(Entity entity, Registry& registry, AssetManager& assetManager) {
    auto& source = registry.RequireComponent<AudioSource>(entity);

    if (source.shouldStop) {
        StopSource(entity, source, assetManager);
    }
    if (source.shouldPlay) {
        StartSource(entity, registry, source, assetManager);
    }
    RefreshSourceState(entity, source, assetManager);
}

void AudioSystem::OnSceneUnloaded(Registry& registry) {
    for (auto& active : m_ActiveSounds) {
        active.sound.Stop();
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

void AudioSystem::Update(Registry& registry, float dt) {
    (void)dt;
    if (!registry.HasResource<AssetManager>()) {
        return;
    }

    auto& assetManager = registry.GetResource<AssetManager>();
    UpdateAssetAudio(assetManager);
    UpdateSoundInstances();

    for (Entity entity = 1; entity < registry.GetEntityCount(); ++entity) {
        if (registry.HasComponent<AudioSource>(entity)) {
            ProcessSource(entity, registry, assetManager);
        }
    }
}

} // namespace Engine
