#include "SceneSerializer.hpp"
#include "Components/Name.hpp"
#include "Logging/Logger.hpp"
#include <filesystem>
#include <fstream>

namespace Serialization {

void SceneSerializer::Serialize(const std::string& filepath, Engine::Registry& registry,
                                const SceneConfig& config, SerializationFormat format) {
    m_Serializer.SetFormat(format);

    auto snapshot = m_Serializer.CaptureScene(registry);

    SceneData sceneData;
    sceneData.sceneName = "Untitled";
    sceneData.config = config;
    sceneData.entities = std::move(snapshot);

    {
        std::error_code ec;
        const std::filesystem::path sceneDir = std::filesystem::path(filepath).parent_path();
        if (!sceneDir.empty() && !config.m_ProjectFolder.empty()) {
            const std::filesystem::path rel =
                std::filesystem::relative(config.m_ProjectFolder, sceneDir, ec);
            if (!ec && !rel.empty()) {
                sceneData.config.m_ProjectFolder = rel.string();
            }
        }
        if (!config.m_InitialScenePath.empty() && !config.m_ProjectFolder.empty()) {
            const std::filesystem::path rel =
                std::filesystem::relative(config.m_InitialScenePath, config.m_ProjectFolder, ec);
            if (!ec && !rel.empty() && rel.string().find("..") == std::string::npos) {
                sceneData.config.m_InitialScenePath = rel.string();
            }
        }
    }

    if (!m_Serializer.SerializeToFile(filepath, sceneData)) {
        log_error("Failed to serialize scene to file: " + filepath);
        return;
    }

    log_info("Successfully saved scene to " + filepath);
}

bool SceneSerializer::Deserialize(const std::string& filepath, Engine::Registry& registry,
                                  std::optional<std::reference_wrapper<SceneConfig>> config,
                                  SerializationFormat format) {
    m_Serializer.SetFormat(format);

    SceneData sceneData;
    if (!m_Serializer.DeserializeFromFile(filepath, sceneData)) {
        log_error("Failed to deserialize scene from file: " + filepath);
        return false;
    }

    log_info("Loaded scene: " + sceneData.sceneName);

    if (config.has_value()) {
        SceneConfig& out = config->get();
        const std::string localFolder = out.m_ProjectFolder;
        const std::string localScene = out.m_InitialScenePath;
        out = sceneData.config;
        out.m_ProjectFolder = localFolder;
        out.m_InitialScenePath = localScene;
    }

    if (!registry.HasResource<ProjectContext>()) {
        std::string folder;
        const std::string& stored = sceneData.config.m_ProjectFolder;
        if (!stored.empty()) {
            std::error_code ec;
            std::filesystem::path candidate(stored);
            const std::filesystem::path sceneDir = std::filesystem::path(filepath).parent_path();
            if (!candidate.is_absolute() && !sceneDir.empty()) {
                candidate = sceneDir / candidate;
            }
            if (std::filesystem::exists(candidate, ec) && !ec) {
                std::string normal = candidate.lexically_normal().string();
                while (normal.size() > 1 && normal.back() == '/') {
                    normal.pop_back();
                }
                folder = normal;
            }
        }
        if (folder.empty()) {
            std::error_code ec;
            folder = std::filesystem::current_path(ec).string();
            if (ec) {
                folder.clear();
            }
        }
        registry.AddResource<ProjectContext>(folder);
    }

    m_Serializer.RestoreScene(registry, sceneData.entities);

    return true;
}

void SceneSerializer::SetFormat(SerializationFormat format) {
    m_Serializer.SetFormat(format);
}

SerializationFormat SceneSerializer::GetFormat() const {
    return m_Serializer.GetFormat();
}

} // namespace Serialization
