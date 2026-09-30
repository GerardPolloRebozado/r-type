#ifndef ENGINE_CORE_ASSETMANAGER_HPP_
#define ENGINE_CORE_ASSETMANAGER_HPP_

#include "Sound.hpp"
#include "Sprites.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <utility>

namespace Engine {
    /**
     * @class AssetManager
     * @brief Singleton class to load, store, and manage game audio assets.
     */
    class AssetManager {
    public:
        class FileNotFoundException : public std::exception {
            std::string _file;

        public:
            const char *what() const noexcept override {
                static std::string msg;
                msg = "The file: " + _file + " was not found";
                return msg.c_str();
            }

            FileNotFoundException(std::string file) : _file(std::move(file)) {
            }
        };

        static AssetManager &GetInstance() {
            static AssetManager instance;
            return instance;
        }

        AssetManager(const AssetManager &) = delete;

        AssetManager &operator=(const AssetManager &) = delete;

        AssetManager(AssetManager &&) = delete;

        AssetManager &operator=(AssetManager &&) = delete;

        void LoadSound(const std::string &alias, const std::string &filePath, bool isLoop = false);

        Sound &GetSound(const std::string &alias);

        void UnloadSound(const std::string &alias);

        void LoadSprite(const std::string &alias, const std::string &filePath);

        Sprite &GetSprite(const std::string &alias);

        void UnloadSprite(const std::string &alias);

        void ClearAll();

    private:
        AssetManager() = default;

        ~AssetManager() {
            ClearAll();
        }

        std::unordered_map<std::string, std::unique_ptr<Sound> > m_Sounds;
        std::unordered_map<std::string, std::unique_ptr<Sprite> > m_Sprites;
    };
} // namespace Engine

#endif // ENGINE_CORE_ASSETMANAGER_HPP_
