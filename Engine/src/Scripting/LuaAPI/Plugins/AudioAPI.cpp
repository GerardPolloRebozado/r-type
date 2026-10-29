#include "Scripting/LuaAPI/Plugins/AudioAPI.hpp"
#include "Assets/AssetManager.hpp"
#include "Assets/ProjectPaths.hpp"
#include "Components/AudioSource.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <algorithm>
#include <exception>
#include <memory>
#include <ranges>
#include <sol/sol.hpp>
#include <string>

namespace Engine::LuaAPI {
namespace {

std::shared_ptr<Sound> getOrLoadSound(Registry& registry, const std::string& path, bool loop) {
    if (path.empty() || !registry.HasResource<AssetManager>()) {
        return nullptr;
    }
    auto& assetManager = registry.GetResource<AssetManager>();
    try {
        return assetManager.GetSound(path);
    } catch (...) {
    }
    try {
        assetManager.LoadSound(path, ResolveAssetPath(registry, path).string(), loop);
        return assetManager.GetSound(path);
    } catch (const std::exception& error) {
        log_error(std::string("[LuaAPI] Sound load failed for '") + path + "': " + error.what());
        return nullptr;
    }
}

std::shared_ptr<Music> getOrLoadMusic(Registry& registry, const std::string& path, bool loop) {
    if (path.empty() || !registry.HasResource<AssetManager>()) {
        return nullptr;
    }
    auto& assetManager = registry.GetResource<AssetManager>();
    try {
        return assetManager.GetMusic(path);
    } catch (...) {
    }
    try {
        assetManager.LoadMusic(path, ResolveAssetPath(registry, path).string(), loop);
        return assetManager.GetMusic(path);
    } catch (const std::exception& error) {
        log_error(std::string("[LuaAPI] Music load failed for '") + path + "': " + error.what());
        return nullptr;
    }
}

} // namespace

void registerAudio(sol::state_view lua, Registry& registry) {
    sol::table audio = Internal::ensureNamespace(lua, "Audio");
    sol::table component = Internal::ensureNamespace(lua, "Component");

    audio["playSound"] = [&registry](const std::string& path, sol::optional<float> volumeOpt,
                                     sol::optional<bool> loopOpt) -> bool {
        auto sound = getOrLoadSound(registry, path, loopOpt.value_or(false));
        if (!sound) {
            return false;
        }
        sound->SetLooping(loopOpt.value_or(false));
        sound->SetVolume(std::clamp(volumeOpt.value_or(1.0f), 0.0f, 1.0f));
        sound->Play();
        return true;
    };

    audio["playMusic"] = [&registry](const std::string& path, sol::optional<float> volumeOpt,
                                     sol::optional<bool> loopOpt) -> bool {
        auto music = getOrLoadMusic(registry, path, loopOpt.value_or(false));
        if (!music) {
            return false;
        }
        music->SetLooping(loopOpt.value_or(false));
        music->SetVolume(std::clamp(volumeOpt.value_or(1.0f), 0.0f, 1.0f));
        music->Play();
        return true;
    };

    audio["stopSound"] = [&registry](const std::string& path) {
        if (!registry.HasResource<AssetManager>()) {
            return;
        }
        try {
            registry.GetResource<AssetManager>().GetSound(path)->Stop();
        } catch (...) {
        }
    };

    audio["stopMusic"] = [&registry](const std::string& path) {
        if (!registry.HasResource<AssetManager>()) {
            return;
        }
        try {
            registry.GetResource<AssetManager>().GetMusic(path)->Stop();
        } catch (...) {
        }
    };

    audio["stopAllSounds"] = [&registry]() {
        if (!registry.HasResource<AssetManager>()) {
            return;
        }
        for (const auto& sound :
             registry.GetResource<AssetManager>().GetAllSounds() | std::views::values) {
            if (sound) {
                sound->Stop();
            }
        }
    };

    audio["stopAllMusic"] = [&registry]() {
        if (!registry.HasResource<AssetManager>()) {
            return;
        }
        for (const auto& music :
             registry.GetResource<AssetManager>().GetAllMusic() | std::views::values) {
            if (music) {
                music->Stop();
            }
        }
    };

    audio["setSoundVolume"] = [&registry](const std::string& path, float volume) -> bool {
        auto sound = getOrLoadSound(registry, path, false);
        if (!sound) {
            return false;
        }
        sound->SetVolume(std::clamp(volume, 0.0f, 1.0f));
        return true;
    };

    audio["setMusicVolume"] = [&registry](const std::string& path, float volume) -> bool {
        auto music = getOrLoadMusic(registry, path, false);
        if (!music) {
            return false;
        }
        music->SetVolume(std::clamp(volume, 0.0f, 1.0f));
        return true;
    };

    audio["isSoundPlaying"] = [&registry](const std::string& path) -> bool {
        try {
            return registry.GetResource<AssetManager>().GetSound(path)->IsPlaying();
        } catch (...) {
            return false;
        }
    };

    audio["isMusicPlaying"] = [&registry](const std::string& path) -> bool {
        try {
            return registry.GetResource<AssetManager>().GetMusic(path)->IsPlaying();
        } catch (...) {
            return false;
        }
    };

    component["addAudioSource"] =
        [&registry](Entity entity, const std::string& path, sol::optional<bool> isMusicOpt,
                    sol::optional<bool> loopOpt, sol::optional<float> volumeOpt) -> bool {
        AudioSource source;
        source.assetId = path;
        source.isMusic = isMusicOpt.value_or(false);
        source.loop = loopOpt.value_or(false);
        source.volume = std::clamp(volumeOpt.value_or(1.0f), 0.0f, 1.0f);
        if (!registry.IsComponentRegistered<AudioSource>()) {
            registry.RegisterComponent<AudioSource>();
        }
        registry.AddComponent<AudioSource>(entity, source);
        return true;
    };

    component["getAudioSource"] = [&registry](sol::this_state ts, Entity entity) -> sol::object {
        sol::state_view view(ts);
        if (!registry.IsComponentRegistered<AudioSource>() ||
            !registry.HasComponent<AudioSource>(entity)) {
            return sol::make_object(view, sol::lua_nil);
        }
        const auto& source = registry.RequireComponent<AudioSource>(entity);
        sol::table t = view.create_table();
        t["path"] = source.assetId;
        t["isMusic"] = source.isMusic;
        t["loop"] = source.loop;
        t["volume"] = source.volume;
        t["isPlaying"] = source.isPlaying;
        return sol::make_object(view, t);
    };

    component["setAudioSource"] =
        [&registry](Entity entity, const std::string& path, sol::optional<bool> isMusicOpt,
                    sol::optional<bool> loopOpt, sol::optional<float> volumeOpt) -> bool {
        if (!registry.IsComponentRegistered<AudioSource>() ||
            !registry.HasComponent<AudioSource>(entity)) {
            return false;
        }
        auto& source = registry.RequireComponent<AudioSource>(entity);

        if (!source.assetId.empty() && source.isMusic && registry.HasResource<AssetManager>()) {
            try {
                registry.GetResource<AssetManager>().GetMusic(source.assetId)->Stop();
            } catch (...) {
            }
        }

        source.assetId = path;
        if (isMusicOpt.has_value()) {
            source.isMusic = isMusicOpt.value();
        }
        if (loopOpt.has_value()) {
            source.loop = loopOpt.value();
        }
        if (volumeOpt.has_value()) {
            source.volume = std::clamp(volumeOpt.value(), 0.0f, 1.0f);
        }
        source.shouldPlay = false;
        source.shouldStop = true;
        source.isPlaying = false;
        return true;
    };

    component["playAudio"] = [&registry](Entity entity) -> bool {
        if (!registry.IsComponentRegistered<AudioSource>() ||
            !registry.HasComponent<AudioSource>(entity)) {
            return false;
        }
        auto& source = registry.RequireComponent<AudioSource>(entity);
        source.shouldPlay = true;
        source.shouldStop = false;
        return true;
    };

    component["stopAudio"] = [&registry](Entity entity) -> bool {
        if (!registry.IsComponentRegistered<AudioSource>() ||
            !registry.HasComponent<AudioSource>(entity)) {
            return false;
        }
        auto& source = registry.RequireComponent<AudioSource>(entity);
        source.shouldPlay = false;
        source.shouldStop = true;
        return true;
    };

    component["setAudioVolume"] = [&registry](Entity entity, float volume) -> bool {
        if (!registry.IsComponentRegistered<AudioSource>() ||
            !registry.HasComponent<AudioSource>(entity)) {
            return false;
        }
        registry.RequireComponent<AudioSource>(entity).volume = std::clamp(volume, 0.0f, 1.0f);
        return true;
    };

    component["setAudioLoop"] = [&registry](Entity entity, bool loop) -> bool {
        if (!registry.IsComponentRegistered<AudioSource>() ||
            !registry.HasComponent<AudioSource>(entity)) {
            return false;
        }
        registry.RequireComponent<AudioSource>(entity).loop = loop;
        return true;
    };
}

} // namespace Engine::LuaAPI
