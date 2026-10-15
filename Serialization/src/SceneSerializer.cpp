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

    if (!m_Serializer.SerializeToFile(filepath, sceneData)) {
        log_error("Failed to serialize scene to file: " + filepath);
        return;
    }

    log_info("Successfully saved scene to " + filepath);
}

bool SceneSerializer::Deserialize(const std::string& filepath, Engine::Registry& registry,
                                  const std::optional<std::reference_wrapper<SceneConfig>> config,
                                  const SerializationFormat format) {
    m_Serializer.SetFormat(format);

    std::error_code ec;
    const std::string fallbackFolder =
        std::filesystem::absolute(filepath, ec).parent_path().string();
    auto registerContext = [&](const std::string& folder) {
        if (!registry.HasResource<ProjectContext>()) {
            registry.AddResource<ProjectContext>(folder);
        } else {
            registry.GetResource<ProjectContext>().projectFolder = folder;
        }
    };

    if (!std::filesystem::exists(filepath, ec) || ec) {
        log_info("Scene file not found, starting with an empty scene: " + filepath);
        registerContext(fallbackFolder);
        if (config.has_value()) {
            config->get().m_ProjectFolder = fallbackFolder;
        }
        return true;
    }

    SceneData sceneData;
    std::string error;
    if (!m_Serializer.DeserializeFromFile(filepath, sceneData, &error)) {
        log_error("Failed to deserialize scene from file: " + filepath +
                  (error.empty() ? "" : " (" + error + ")"));
        registerContext(fallbackFolder);
        return false;
    }

    log_info("Loaded scene: " + sceneData.sceneName);

    const std::string& projectFolder = sceneData.config.m_ProjectFolder;

    if (config.has_value()) {
        config->get() = sceneData.config;
    }

    // ProjectContext must be registered even when no SceneConfig is passed in
    // (e.g. built games), so relative asset/script paths resolve correctly
    if (!registry.HasResource<ProjectContext>()) {
        registry.AddResource<ProjectContext>(projectFolder);
    } else {
        registry.GetResource<ProjectContext>().projectFolder = projectFolder;
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