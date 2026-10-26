#ifndef ENGINE_CORE_MUSIC_HPP_
#define ENGINE_CORE_MUSIC_HPP_

#include <algorithm>
#include <raylib-cpp.hpp>
#include <string>

namespace Engine {
/**
 * @class Music
 * @brief Engine wrapper for audio streaming (long tracks). Cannot be cloned.
 */
class Music {
  private:
    raylib::Music _music;
    float _volume = 1.0f;

  public:
    // Default constructor for ECS
    Music() = default;

    Music(const std::string& path, bool isLoop = false) {
        _music = raylib::Music(path);
        _music.Stop();
        _music.SetLooping(isLoop);
        _music.SetVolume(_volume);
    }

    Music(const char* fileType, const unsigned char* data, const int dataSize,
          const bool isLoop = false) {
        _music = raylib::Music(LoadMusicStreamFromMemory(fileType, data, dataSize));
        _music.Stop();
        _music.SetLooping(isLoop);
        _music.SetVolume(_volume);
    }

    // Music streams cannot be easily cloned because they read from disk not from memory, they WON'T
    // BE CLONED
    Music(const Music&) = delete;

    Music& operator=(const Music&) = delete;

    Music(Music&& other) = default;

    Music& operator=(Music&& other) = default;

    void Play() {
        _music.SetVolume(_volume);
        _music.Play();
    }

    void Update() {
        _music.Update();
    }

    bool IsPlaying() const {
        return _music.IsPlaying();
    }

    void Stop() {
        _music.Stop();
    }

    bool IsLooping() const {
        return _music.GetLooping();
    }

    void SetLooping(bool value) {
        _music.SetLooping(value);
    }

    void SetVolume(const float volume) {
        _volume = std::clamp(volume, 0.0f, 1.0f);
        _music.SetVolume(_volume);
    }

    float GetVolume() const {
        return _volume;
    }
};
} // namespace Engine

#endif // ENGINE_CORE_MUSIC_HPP_
