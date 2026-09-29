#ifndef ENGINE_CORE_SOUND_HPP_
#define ENGINE_CORE_SOUND_HPP_

#include <raylib-cpp.hpp>
#include <string>

namespace Engine {
    /**
     * @class Sound
     * @brief Engine wrapper for audio, utilizing raylib::Music to support streaming and looping.
     */
    class Sound {
    private:
        raylib::Music _music;

    public:
        Sound(const std::string &path, bool isLoop = false) {
            _music = raylib::Music(path);
            _music.Stop();
            _music.SetLooping(isLoop);
        }

        void Play() {
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
    };
} // namespace Engine

#endif // ENGINE_CORE_SOUND_HPP_
