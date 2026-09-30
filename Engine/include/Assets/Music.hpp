#ifndef ENGINE_CORE_MUSIC_HPP_
#define ENGINE_CORE_MUSIC_HPP_

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

    public:
        // Default constructor for ECS
        Music() = default;

        Music(const std::string &path, bool isLoop = false) {
            _music = raylib::Music(path);
            _music.Stop();
            _music.SetLooping(isLoop);
        }

        Music(const char *fileType, const unsigned char *data, const int dataSize, const bool isLoop = false) {
            _music = raylib::Music(LoadMusicStreamFromMemory(fileType, data, dataSize));
            _music.Stop();
            _music.SetLooping(isLoop);
        }

        // Music streams cannot be easily cloned because they read from disk not from memory, they WON'T BE CLONED
        Music(const Music &) = delete;

        Music &operator=(const Music &) = delete;

        Music(Music &&other) = default;

        Music &operator=(Music &&other) = default;

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

#endif // ENGINE_CORE_MUSIC_HPP_
