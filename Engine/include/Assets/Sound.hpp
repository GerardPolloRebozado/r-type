#ifndef ENGINE_CORE_SOUND_HPP_
#define ENGINE_CORE_SOUND_HPP_

#include <raylib-cpp.hpp>
#include <string>

namespace Engine {
    /**
     * @class Sound
     * @brief Engine wrapper for audio
     */
    class Sound {
        raylib::SoundUnmanaged _sound;
        bool _isLooping;
        bool _isAlias;

    public:
        Sound() : _isLooping(false), _isAlias(false) {
        }

        Sound(const std::string &path, const bool isLoop = false) : _isLooping(isLoop), _isAlias(false) {
            _sound = raylib::SoundUnmanaged(path);
        }

        Sound(const char *fileType, const unsigned char *data, int dataSize,
              const bool isLoop = false) : _isLooping(isLoop), _isAlias(false) {
            const Wave wave = LoadWaveFromMemory(fileType, data, dataSize);
            _sound = raylib::SoundUnmanaged(LoadSoundFromWave(wave));
            UnloadWave(wave);
        }

        ~Sound() {
            if (_sound.frameCount > 0) {
                if (_isAlias) {
                    UnloadSoundAlias(_sound);
                } else {
                    UnloadSound(_sound);
                }
                _sound.frameCount = 0;
            }
        }

        Sound(const Sound &other) : _isLooping(other._isLooping), _isAlias(true) {
            if (other._sound.frameCount > 0) {
                const auto alias = LoadSoundAlias(other._sound);
                _sound = raylib::SoundUnmanaged(alias.stream, alias.frameCount);
            }
        }

        Sound(Sound &&other) noexcept : _sound(other._sound), _isLooping(other._isLooping), _isAlias(other._isAlias) {
            other._sound = raylib::SoundUnmanaged();
            other._sound.frameCount = 0;
            other._isAlias = false;
        }

        Sound &operator=(const Sound &other) {
            if (this != &other) {
                if (_sound.frameCount > 0) {
                    if (_isAlias) UnloadSoundAlias(_sound);
                    else UnloadSound(_sound);
                }

                _isLooping = other._isLooping;
                _isAlias = true;

                if (other._sound.frameCount > 0) {
                    const auto alias = LoadSoundAlias(other._sound);
                    _sound = raylib::SoundUnmanaged(alias.stream, alias.frameCount);
                } else
                    _sound = raylib::SoundUnmanaged();
            }
            return *this;
        }

        Sound &operator=(Sound &&other) noexcept {
            if (this != &other) {
                if (_sound.frameCount > 0) {
                    if (_isAlias)
                        UnloadSoundAlias(_sound);
                    else
                        UnloadSound(_sound);
                }

                _sound = other._sound;
                _isLooping = other._isLooping;
                _isAlias = other._isAlias;

                other._sound = raylib::SoundUnmanaged();
                other._sound.frameCount = 0;
                other._isAlias = false;
            }
            return *this;
        }

        void Play() {
            if (_sound.frameCount > 0) {
                _sound.Play();
            }
        }

        bool IsPlaying() const {
            return _sound.frameCount > 0 && _sound.IsPlaying();
        }

        void Stop() {
            if (_sound.frameCount > 0) {
                _sound.Stop();
            }
        }

        bool IsLooping() const {
            return _isLooping;
        }

        void SetLooping(const bool value) {
            _isLooping = value;
        }
    };
} // namespace Engine

#endif // ENGINE_CORE_SOUND_HPP_
