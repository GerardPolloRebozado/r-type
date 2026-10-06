#include "Assets/AssetManager.hpp"
#include "Errors/AssetErrors.hpp"
#include "Logging/Logger.hpp"
#include <raylib-cpp.hpp>

namespace Engine {
void AssetManager::LoadSound(const std::string& alias, const std::string& filePath, bool isLoop) {
    if (m_Sounds.contains(alias)) {
        return;
    }

    try {
        m_Sounds[alias] = std::make_unique<Sound>(filePath, isLoop);
    } catch (...) {
        throw ErrorAsset("The file: " + filePath + " was not found");
    }
}

void AssetManager::LoadSoundFromMemory(const std::string& alias, const char* fileType,
                                       const unsigned char* data, int dataSize, bool isLoop) {
    if (m_Sounds.contains(alias)) {
        return;
    }

    try {
        m_Sounds[alias] = std::make_unique<Sound>(fileType, data, dataSize, isLoop);
    } catch (...) {
        throw ErrorAsset("Failed to load sound from memory: " + alias);
    }
}

std::shared_ptr<Sound> AssetManager::GetSound(const std::string& alias) {
    const auto it = m_Sounds.find(alias);
    if (it == m_Sounds.end()) {
        throw ErrorAsset("Sound not found: " + alias);
    }
    return it->second;
}

void AssetManager::UnloadSound(const std::string& alias) {
    const auto it = m_Sounds.find(alias);
    if (it != m_Sounds.end()) {
        m_Sounds.erase(it);
        log_info("[AssetManager] Unloaded sound: " + alias);
    }
}

void AssetManager::LoadSprite(const std::string& alias, const std::string& filePath) {
    if (m_Sprites.contains(alias)) {
        return;
    }

    try {
        m_Sprites[alias] = std::make_unique<Sprite>(filePath);
    } catch (...) {
        throw ErrorAsset("The file: " + filePath + " was not found");
    }
}

std::shared_ptr<Sprite> AssetManager::GetSprite(const std::string& alias) {
    const auto it = m_Sprites.find(alias);
    if (it == m_Sprites.end()) {
        throw ErrorAsset("Sprite not found: " + alias);
    }
    return it->second;
}

void AssetManager::UnloadSprite(const std::string& alias) {
    const auto it = m_Sprites.find(alias);
    if (it != m_Sprites.end()) {
        m_Sprites.erase(it);
        log_info("[AssetManager] Unloaded sprite: " + alias);
    }
}

void AssetManager::ClearAll() {
    m_Sounds.clear();
    m_Sprites.clear();
    m_Musics.clear();
    log_info("[AssetManager] Cleared all assets.");
}

void AssetManager::LoadFontFromMemory(const std::string& alias, const std::string& fileType,
                                      const unsigned char* fileData, int dataSize, int fontSize,
                                      int* fontChars, int charsCount) {
    if (m_Fonts.contains(alias)) {
        return;
    }
    m_Fonts[alias] = std::make_unique<raylib::Font>(fileType, fileData, dataSize, fontSize,
                                                    fontChars, charsCount);
}

std::shared_ptr<raylib::Font> AssetManager::GetFont(const std::string& alias) {
    const auto it = m_Fonts.find(alias);
    if (it == m_Fonts.end()) {
        throw ErrorAsset("Font not found: " + alias);
    }
    return it->second;
}

void AssetManager::LoadMusic(const std::string& alias, const std::string& filePath, bool isLoop) {
    if (m_Musics.contains(alias)) {
        return;
    }
    m_Musics[alias] = std::make_unique<Music>(filePath, isLoop);
}

void AssetManager::LoadMusicFromMemory(const std::string& alias, const char* fileType,
                                       const unsigned char* data, int dataSize, bool isLoop) {
    if (m_Musics.contains(alias)) {
        return;
    }
    m_Musics[alias] = std::make_unique<Music>(fileType, data, dataSize, isLoop);
}

std::shared_ptr<Music> AssetManager::GetMusic(const std::string& alias) {
    const auto it = m_Musics.find(alias);
    if (it == m_Musics.end()) {
        throw ErrorAsset("Music not found: " + alias);
    }
    return it->second;
}

void AssetManager::UnloadMusic(const std::string& alias) {
    const auto it = m_Musics.find(alias);
    if (it != m_Musics.end()) {
        m_Musics.erase(it);
        log_info("[AssetManager] Unloaded music: " + alias);
    }
}
} // namespace Engine
