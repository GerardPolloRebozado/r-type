#include "Serialization/SceneSerializer.hpp"
#include "EditorConfig.hpp"
#include "Logging/Logger.hpp"
#include "Plugins/PositionPluginEditor.hpp"
#include "Plugins/TextPluginEditor.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Editor {

void SceneSerializer::Serialize(const std::string& filepath, Engine::Registry& registry,
                                EditorConfig* config) {
    json sceneJson;
    sceneJson["SceneName"] = "Untitled";

    if (config) {
        json projectConfiguration;
        projectConfiguration["ProjectFolder"] = config->m_ProjectFolder;
        projectConfiguration["WindowTitle"] = config->windowTitle;
        projectConfiguration["WindowWidth"] = config->windowWidth;
        projectConfiguration["WindowHeight"] = config->windowHeight;
        projectConfiguration["TargetFPS"] = config->targetFPS;
        projectConfiguration["BuildExecutableName"] = config->m_BuildExecutableName;
        projectConfiguration["BuildWindowTitle"] = config->m_BuildWindowTitle;
        sceneJson["Configuration"] = projectConfiguration;
    }

    json entitiesJson = json::array();

    // We instantiate the editors here (later they will be centralized)
    PositionPluginEditor positionEditor;
    TextPluginEditor textEditor;

    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Engine::Entity entity = i;
        json entityJson;
        entityJson["EntityID"] = static_cast<int>(entity);

        positionEditor.Serialize(entity, registry, entityJson);
        textEditor.Serialize(entity, registry, entityJson);

        entitiesJson.push_back(entityJson);
    }

    sceneJson["Entities"] = entitiesJson;

    std::ofstream fout(filepath);
    if (fout.is_open()) {
        fout << sceneJson.dump(4);
        fout.close();
        log_info("Successfully saved scene to " + filepath);
    } else {
        log_error("Failed to open file for writing: " + filepath);
    }
}

bool SceneSerializer::Deserialize(const std::string& filepath, Engine::Registry& registry,
                                  EditorConfig* config) {
    std::ifstream fin(filepath);
    if (!fin.is_open()) {
        log_error("Failed to open file for reading: " + filepath);
        return false;
    }

    json sceneJson;
    fin >> sceneJson;

    log_info("Loaded scene: " + sceneJson.value("SceneName", "Unknown"));

    if (config && sceneJson.contains("Configuration")) {
        auto projectConfiguration = sceneJson["Configuration"];
        if (projectConfiguration.contains("ProjectFolder")) {
            config->m_ProjectFolder = projectConfiguration["ProjectFolder"];
        }
        if (projectConfiguration.contains("WindowTitle")) {
            config->windowTitle = projectConfiguration["WindowTitle"];
        }
        if (projectConfiguration.contains("WindowWidth")) {
            config->windowWidth = projectConfiguration["WindowWidth"];
        }
        if (projectConfiguration.contains("WindowHeight")) {
            config->windowHeight = projectConfiguration["WindowHeight"];
        }
        if (projectConfiguration.contains("TargetFPS")) {
            config->targetFPS = projectConfiguration["TargetFPS"];
        }
        if (projectConfiguration.contains("BuildExecutableName")) {
            config->m_BuildExecutableName = projectConfiguration["BuildExecutableName"];
        }
        if (projectConfiguration.contains("BuildWindowTitle")) {
            config->m_BuildWindowTitle = projectConfiguration["BuildWindowTitle"];
        }
    }

    PositionPluginEditor positionEditor;
    TextPluginEditor textEditor;
    if (sceneJson.contains("Entities")) {
        for (const auto& entityJson : sceneJson["Entities"]) {
            Engine::Entity entity = registry.CreateEntity();

            // Deserialize components back onto the newly spawned entity
            positionEditor.Deserialize(entity, registry, entityJson);
            textEditor.Deserialize(entity, registry, entityJson);
        }
    }

    return true;
}

} // namespace Editor