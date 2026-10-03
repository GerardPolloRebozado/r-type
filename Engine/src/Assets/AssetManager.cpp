#include "Assets/AssetManager.hpp"
#include <iostream>
#include <raylib-cpp.hpp>

namespace Engine {
    void AssetManager::LoadSound(const std::string &alias, const std::string &filePath, bool isLoop) {
        if (m_Sounds.contains(alias)) {
            return;
        }

        try {
            m_Sounds[alias] = std::make_unique<Sound>(filePath, isLoop);
        } catch (...) {
            throw FileNotFoundException(filePath);
        }
    }

    void AssetManager::LoadSoundFromMemory(const std::string &alias, const char *fileType, const unsigned char *data,
                                           int dataSize, bool isLoop) {
        if (m_Sounds.contains(alias)) {
            return;
        }

        try {
            m_Sounds[alias] = std::make_unique<Sound>(fileType, data, dataSize, isLoop);
        } catch (...) {
            throw std::runtime_error("Failed to load sound from memory: " + alias);
        }
    }

    Sound &AssetManager::GetSound(const std::string &alias) {
        const auto it = m_Sounds.find(alias);
        if (it == m_Sounds.end()) {
            throw std::runtime_error("Sound not found: " + alias);
        }
        return *it->second;
    }

    void AssetManager::UnloadSound(const std::string &alias) {
        const auto it = m_Sounds.find(alias);
        if (it != m_Sounds.end()) {
            m_Sounds.erase(it);
            std::cout << "[AssetManager] Unloaded sound: " << alias << "\n";
        }
    }

    void AssetManager::ClearAll() {
        m_Sounds.clear();
        m_Musics.clear();
        std::cout << "[AssetManager] Cleared all assets.\n";
    }

    void AssetManager::LoadFontFromMemory(const std::string &alias, const std::string &fileType,
                                          const unsigned char *fileData, int dataSize, int fontSize, int *fontChars,
                                          int charsCount) {
        if (m_Fonts.contains(alias)) return;
        m_Fonts[alias] = std::make_unique<raylib::Font>(fileType, fileData, dataSize, fontSize,
                                                        fontChars,
                                                        charsCount);
    }

    raylib::Font &AssetManager::GetFont(const std::string &alias) {
        const auto it = m_Fonts.find(alias);
        if (it == m_Fonts.end()) {
            throw std::runtime_error("Sound not found: " + alias);
        }
        return *it->second;
    }
} // namespace Engine

namespace Engine {
    void AssetManager::LoadMusic(const std::string &alias, const std::string &filePath, bool isLoop) {
        if (m_Musics.contains(alias)) return;
        m_Musics[alias] = std::make_unique<Music>(filePath, isLoop);
    }

    void AssetManager::LoadMusicFromMemory(const std::string &alias, const char *fileType, const unsigned char *data,
                                           int dataSize, bool isLoop) {
        if (m_Musics.contains(alias)) return;
        m_Musics[alias] = std::make_unique<Music>(fileType, data, dataSize, isLoop);
    }

    Music &AssetManager::GetMusic(const std::string &alias) {
        const auto it = m_Musics.find(alias);
        if (it == m_Musics.end()) {
            throw std::runtime_error("Music not found: " + alias);
        }
        return *it->second;
    }

    void AssetManager::UnloadMusic(const std::string &alias) {
        const auto it = m_Musics.find(alias);
        if (it != m_Musics.end()) {
            m_Musics.erase(it);
            std::cout << "[AssetManager] Unloaded music: " << alias << "\n";
        }
    }
}
