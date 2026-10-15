#include "SceneSerializer.hpp"
#include "Components/Name.hpp"
#include "Logging/Logger.hpp"
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

    if (registry.HasResource<Editor::EntityDirectoriesManager>()) {
        sceneData.directories = registry.GetResource<Editor::EntityDirectoriesManager>();
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

    std::unordered_map<Engine::Entity, Engine::Entity> idRemap;
    m_Serializer.RestoreScene(registry, sceneData.entities, &idRemap);

    if (sceneData.directories.directories.empty()) {
        sceneData.directories.directories.push_back({"/", "root", {}});
    }

    for (auto& dir : sceneData.directories.directories) {
        for (auto& entity : dir.entities) {
            auto it = idRemap.find(entity);
            if (it != idRemap.end()) {
                entity = it->second;
            }
        }
    }

    bool anyEntitiesInDirs = false;
    for (const auto& dir : sceneData.directories.directories) {
        if (!dir.entities.empty()) {
            anyEntitiesInDirs = true;
            break;
        }
    }
    if (!anyEntitiesInDirs && !sceneData.entities.entities.empty()) {
        for (const auto& pair : idRemap) {
            sceneData.directories.directories[0].entities.push_back(pair.second);
        }
    }

    if (!registry.HasResource<Editor::EntityDirectoriesManager>()) {
        registry.AddResource<Editor::EntityDirectoriesManager>(std::move(sceneData.directories));
    } else {
        registry.GetResource<Editor::EntityDirectoriesManager>() = std::move(sceneData.directories);
    }

    return true;
}

void SceneSerializer::SetFormat(SerializationFormat format) {
    m_Serializer.SetFormat(format);
}

SerializationFormat SceneSerializer::GetFormat() const {
    return m_Serializer.GetFormat();
}

} // namespace Serialization
