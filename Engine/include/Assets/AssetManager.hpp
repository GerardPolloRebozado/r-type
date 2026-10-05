#ifndef ENGINE_CORE_ASSETMANAGER_HPP_
#define ENGINE_CORE_ASSETMANAGER_HPP_

#include "Music.hpp"
#include "Sound.hpp"
#include "Sprites.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace Engine {
/**
 * @class AssetManager
 * @brief Singleton class to load, store, and manage game audio assets.
 */
class AssetManager {
  public:
    AssetManager() = default;

    ~AssetManager() {
        ClearAll();
    }

    AssetManager(const AssetManager&) = delete;

    AssetManager& operator=(const AssetManager&) = delete;

    AssetManager(AssetManager&&) = delete;

    AssetManager& operator=(AssetManager&&) = delete;

    void LoadSound(const std::string& alias, const std::string& filePath, bool isLoop = false);

    void LoadSoundFromMemory(const std::string& alias, const char* fileType,
                             const unsigned char* data, int dataSize, bool isLoop = false);

    Sound& GetSound(const std::string& alias);

    void UnloadSound(const std::string& alias);

    /**
     * @brief Loads a sprite texture from a file path and assigns it an alias.
     * @param alias The unique identifier for the sprite.
     * @param filePath The file path to the image file.
     */
    void LoadSprite(const std::string& alias, const std::string& filePath);

    /**
     * @brief Retrieves a sprite by its alias.
     * @param alias The unique identifier for the sprite.
     * @return Reference to the Sprite object.
     * @throws ErrorAsset if the sprite alias is not found.
     */
    Sprite& GetSprite(const std::string& alias);

    /**
     * @brief Unloads a sprite and frees its resources.
     * @param alias The unique identifier for the sprite.
     */
    void UnloadSprite(const std::string& alias);

    /**
     * @brief Clears all loaded assets (sounds and sprites).
     */
    void LoadMusic(const std::string& alias, const std::string& filePath, bool isLoop = false);

    void LoadMusicFromMemory(const std::string& alias, const char* fileType,
                             const unsigned char* data, int dataSize, bool isLoop = false);

    Music& GetMusic(const std::string& alias);

    void UnloadMusic(const std::string& alias);

    void ClearAll();

    /**
     *
     * @param alias name to be used to retrieve this font
     * @param fileType Extension of the file like '.tff'
     * @param fileData the font file itself
     * @param dataSize the size of data to load
     * @param fontSize the base size of the font
     * @param fontChars In case we need to load more letters than ASCII like for example Ñ. If its
     * nullptr it will load only ASCII
     * @param charsCount the number of elements to load from fontChars
     */
    void LoadFontFromMemory(const std::string& alias, const std::string& fileType,
                            const unsigned char* fileData, int dataSize, int fontSize = 12,
                            int* fontChars = nullptr, int charsCount = 0);

    raylib::Font& GetFont(const std::string& alias);

    std::unordered_map<std::string, std::unique_ptr<Music>>& GetAllMusic() {
        return m_Musics;
    }

  private:
    std::unordered_map<std::string, std::unique_ptr<Sound>> m_Sounds;
    std::unordered_map<std::string, std::unique_ptr<Sprite>> m_Sprites;
    std::unordered_map<std::string, std::unique_ptr<Music>> m_Musics;
    std::unordered_map<std::string, std::unique_ptr<raylib::Font>> m_Fonts;
};
} // namespace Engine

#endif // ENGINE_CORE_ASSETMANAGER_HPP_
