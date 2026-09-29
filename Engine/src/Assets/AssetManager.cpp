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

    void AssetManager::ClearAll() {
        m_Sounds.clear();
        std::cout << "[AssetManager] Cleared all assets.\n";
    }
} // namespace Engine
