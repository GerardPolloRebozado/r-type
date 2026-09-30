#include "Assets/AssetManager.hpp"
#include <iostream>

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

    void AssetManager::LoadSprite(const std::string &alias, const std::string &filePath) {
        if (m_Sprites.contains(alias)) {
            return;
        }

        try {
            m_Sprites[alias] = std::make_unique<Sprite>(filePath);
        } catch (...) {
            throw FileNotFoundException(filePath);
        }
    }

    Sprite &AssetManager::GetSprite(const std::string &alias) {
        const auto it = m_Sprites.find(alias);
        if (it == m_Sprites.end()) {
            throw std::runtime_error("Sprite not found: " + alias);
        }
        return *it->second;
    }

    void AssetManager::UnloadSprite(const std::string &alias) {
        const auto it = m_Sprites.find(alias);
        if (it != m_Sprites.end()) {
            m_Sprites.erase(it);
            std::cout << "[AssetManager] Unloaded sprite: " << alias << "\n";
        }
    }

    void AssetManager::ClearAll() {
        m_Sounds.clear();
        m_Sprites.clear();
        std::cout << "[AssetManager] Cleared all assets.\n";
    }
} // namespace Engine
