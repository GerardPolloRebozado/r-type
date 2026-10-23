#include "Resources/SceneManager.hpp"
#include "Logging/Logger.hpp"
#include <filesystem>
#include <utility>

namespace Engine {

void SceneManager::Register(const std::string& name, Builder builder) {
    m_CodeScenes.insert_or_assign(name, std::move(builder));
}

bool SceneManager::IsRegistered(const std::string& name) const {
    return m_CodeScenes.contains(name);
}

void SceneManager::SetFileLoader(FileLoader loader) {
    m_FileLoader = std::move(loader);
}

bool SceneManager::HasFileLoader() const {
    return static_cast<bool>(m_FileLoader);
}

void SceneManager::Load(const std::string& name) {
    m_Pending = name;
}

bool SceneManager::HasPendingLoad() const {
    return m_Pending.has_value();
}

const std::string& SceneManager::Current() const {
    return m_Current;
}

void SceneManager::OnLoaded(LoadedCallback callback) {
    m_LoadedCallbacks.push_back(std::move(callback));
}

bool SceneManager::ApplyPending(Registry& registry) {
    if (!m_Pending.has_value()) {
        return false;
    }
    // Take the request first: a builder or callback may call Load() again,
    // and that new request must wait for the next frame.
    const std::string name = std::move(*m_Pending);
    m_Pending.reset();

    // Check the scene exists before destroying the current one.
    const auto codeScene = m_CodeScenes.find(name);
    const bool isCodeScene = codeScene != m_CodeScenes.end();
    if (!isCodeScene) {
        if (!m_FileLoader) {
            log_error("[Scene] Unknown scene '" + name +
                      "': not a code scene and no scene file loader is installed.");
            return false;
        }
        std::error_code error;
        if (!std::filesystem::exists(name, error) || error) {
            log_error("[Scene] Scene file not found: " + name);
            return false;
        }
    }

    // Same order as System::Update. Copies, because OnSceneUnloaded could
    // register a new system and invalidate iterators.
    const auto order = registry.GetSystemUpdateOrder();
    const auto systems = registry.GetSystems();
    for (const auto& systemId : order) {
        systems.at(systemId)->OnSceneUnloaded(registry);
    }
    registry.ClearEntities();

    if (isCodeScene) {
        // Copy the builder: it may call Register() and replace itself.
        const Builder builder = codeScene->second;
        builder(registry);
    } else if (!m_FileLoader(registry, name)) {
        log_error("[Scene] Failed to load scene file: " + name);
        m_Current.clear();
        return true;
    }

    m_Current = name;
    log_info("[Scene] Loaded scene: " + name);

    const auto callbacks = m_LoadedCallbacks;
    for (const auto& callback : callbacks) {
        callback(registry, name);
    }
    return true;
}

} // namespace Engine
