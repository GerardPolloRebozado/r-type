#include "Serialization/SceneSerializer.hpp"
#include "Components/Name.hpp"
#include "EditorConfig.hpp"
#include "Gallery.hpp"
#include "Logging/Logger.hpp"
#include "MultiplatformUtils.hpp"
#include "Plugins/PositionPluginEditor.hpp"
#include "Plugins/RotationPluginEditor.hpp"
#include "Plugins/ShapePluginEditor.hpp"
#include "Plugins/SpritePluginEditor.hpp"
#include "Plugins/TextPluginEditor.hpp"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Editor {

std::int64_t UnixTimestampSeconds() {
    return std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

void SceneSerializer::UpdateProjectList(EditorConfig& config) {
    auto projects = Gallery::FindProjects();

    auto it = std::find_if(projects.begin(), projects.end(), [&config](const ProjectData& project) {
        return project.path == config.m_ProjectFolder;
    });

    if (it == projects.end()) {
        return;
    }

    it->lastOpened = std::to_string(UnixTimestampSeconds());

    nlohmann::json data = nlohmann::json::array();
    for (const auto& project : projects) {
        data.push_back({{"name", project.name},
                        {"description", project.description},
                        {"lastOpened", project.lastOpened},
                        {"path", project.path},
                        {"engineVersion", project.engineVersion}});
    }

    const auto settingsPath = ConfigDirectory() / "projects.json";
    std::ofstream file(settingsPath);
    if (!file) {
        throw std::runtime_error("Could not open " + settingsPath.string());
    }

    file << data.dump(2) << '\n';
}

void SceneSerializer::Serialize(const std::string& filepath, Engine::Registry& registry,
                                EditorConfig& config) {
    json sceneJson;
    sceneJson["SceneName"] = "Untitled";

    json projectConfiguration;
    projectConfiguration["ProjectFolder"] = config.m_ProjectFolder;
    projectConfiguration["WindowTitle"] = config.windowTitle;
    projectConfiguration["WindowWidth"] = config.windowWidth;
    projectConfiguration["WindowHeight"] = config.windowHeight;
    projectConfiguration["TargetFPS"] = config.targetFPS;
    projectConfiguration["BuildExecutableName"] = config.m_BuildExecutableName;
    projectConfiguration["BuildWindowTitle"] = config.m_BuildWindowTitle;
    sceneJson["Configuration"] = projectConfiguration;

    json inputMapJson = json::object();
    for (const auto& [action, keybinds] : config.m_InputMap) {
        json keybindsJson = json::array();
        for (const auto& kb : keybinds) {
            json kbJson;
            kbJson["key"] = kb.key;
            kbJson["composeKey"] = kb.composeKey;
            kbJson["type"] = static_cast<int>(kb.type);
            keybindsJson.push_back(kbJson);
        }
        inputMapJson[action] = keybindsJson;
    }

    std::string inputFilePath = config.m_ProjectFolder + "/input.json";
    std::ofstream inputOut(inputFilePath);
    if (inputOut.is_open()) {
        inputOut << inputMapJson.dump(4);
        inputOut.close();
    }

    json entitiesJson = json::array();

    // We instantiate the editors here (later they will be centralized)
    PositionPluginEditor positionEditor;
    TextPluginEditor textEditor;
    SpritePluginEditor spriteEditor;
    ShapePluginEditor shapeEditor;
    RotationPluginEditor rotationEditor;

    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Engine::Entity entity = i;
        json entityJson;
        entityJson["EntityID"] = static_cast<int>(entity);
        if (registry.HasComponent<Engine::Name>(entity)) {
            entityJson["Name"] = registry.GetComponent<Engine::Name>(entity).value;
        }

        positionEditor.Serialize(entity, registry, entityJson);
        textEditor.Serialize(entity, registry, entityJson);
        spriteEditor.Serialize(entity, registry, entityJson);
        shapeEditor.Serialize(entity, registry, entityJson);
        rotationEditor.Serialize(entity, registry, entityJson);

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
                                  EditorConfig& config) {
    std::ifstream fin(filepath);
    if (!fin.is_open()) {
        log_error("Failed to open file for reading: " + filepath);
        return false;
    }

    json sceneJson;
    fin >> sceneJson;

    log_info("Loaded scene: " + sceneJson.value("SceneName", "Unknown"));

    if (sceneJson.contains("Configuration")) {
        auto projectConfiguration = sceneJson["Configuration"];
        if (projectConfiguration.contains("ProjectFolder")) {
            config.m_ProjectFolder = projectConfiguration["ProjectFolder"];
        }
        if (projectConfiguration.contains("WindowTitle")) {
            config.windowTitle = projectConfiguration["WindowTitle"];
        }
        if (projectConfiguration.contains("WindowWidth")) {
            config.windowWidth = projectConfiguration["WindowWidth"];
        }
        if (projectConfiguration.contains("WindowHeight")) {
            config.windowHeight = projectConfiguration["WindowHeight"];
        }
        if (projectConfiguration.contains("TargetFPS")) {
            config.targetFPS = projectConfiguration["TargetFPS"];
        }
        if (projectConfiguration.contains("BuildExecutableName")) {
            config.m_BuildExecutableName = projectConfiguration["BuildExecutableName"];
        }
        if (projectConfiguration.contains("BuildWindowTitle")) {
            config.m_BuildWindowTitle = projectConfiguration["BuildWindowTitle"];
        }
    }

    std::string inputFilePath = config.m_ProjectFolder + "/input.json";
    std::ifstream inputIn(inputFilePath);
    if (inputIn.is_open()) {
        json inputMapJson;
        inputIn >> inputMapJson;
        config.m_InputMap.clear();
        for (auto it = inputMapJson.begin(); it != inputMapJson.end(); ++it) {
            std::vector<Engine::Keybind> keybinds;
            for (const auto& kbJson : it.value()) {
                Engine::Keybind kb;
                kb.key = kbJson.value("key", 0);
                kb.composeKey = kbJson.value("composeKey", 0);
                kb.type = static_cast<Engine::DeviceType>(kbJson.value("type", 0));
                keybinds.push_back(kb);
            }
            config.m_InputMap[it.key()] = keybinds;
        }
    }

    PositionPluginEditor positionEditor;
    TextPluginEditor textEditor;
    SpritePluginEditor spriteEditor;
    ShapePluginEditor shapeEditor;
    RotationPluginEditor rotationEditor;
    if (sceneJson.contains("Entities")) {
        for (const auto& entityJson : sceneJson["Entities"]) {
            Engine::Entity entity = registry.CreateEntity();

            if (entityJson.contains("Name") && entityJson["Name"].is_string()) {
                registry.AddComponent<Engine::Name>(
                    entity, Engine::Name(entityJson["Name"].get<std::string>()));
            }
            // Deserialize components back onto the newly spawned entity
            positionEditor.Deserialize(entity, registry, entityJson);
            textEditor.Deserialize(entity, registry, entityJson);
            spriteEditor.Deserialize(entity, registry, entityJson);
            shapeEditor.Deserialize(entity, registry, entityJson);
            rotationEditor.Deserialize(entity, registry, entityJson);
        }
    }

    return true;
}

} // namespace Editor
