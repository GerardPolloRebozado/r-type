#ifndef PLUGINS_SOUND_SOUNDPLUGIN_HPP_
#define PLUGINS_SOUND_SOUNDPLUGIN_HPP_

#include "raylib-cpp.hpp"
#include "Plugins/IPlugin.hpp"
#include <string>
#include <utility>

namespace Engine {
    /**
     * @class SoundPlugin
     * @brief Sound plugin that allow to play, stop and loop a sound
     */
    class SoundPlugin : public IPlugin {
        raylib::Music _music;

    public:
        class FileNotFoundException : std::exception {
            std::string _file;

        public:
            const char *what() const noexcept override {
                return (std::string("The file: ") + _file + std::string("was not found")).c_str();
            }

            FileNotFoundException(std::string file) : _file(std::move(file)) {
            };
        };

        SoundPlugin(const std::string &path, bool isLoop = false) {
            try {
                _music = raylib::Music(path);
                _music.SetLooping(isLoop);
            } catch (const std::exception &) {
                throw FileNotFoundException(path);
            }
        }

        void Play() {
            _music.Play();
        }

        bool isPlaying() const {
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

        ~SoundPlugin() override = default;

        /**
         * @brief Initializes the sound plugin with the ECS regist            _sound = Sound(path);
ry.
         * @param registry A reference to the main ECS registry.
         */
        void Init(Registry &registry) override;
    };
} // namespace Engine

#endif // PLUGINS_SOUND_SOUNDPLUGIN_HPP_
