#include "SceneSerializer.hpp"
#include "ComponentSerializer.hpp"
#include "Components/Name.hpp"
#include "Logging/Logger.hpp"
#include "PathUtils.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Serialization {

void SceneSerializer::Serialize(const std::string& filepath, Engine::Registry& registry,
                                const SceneConfig& config) {
    json sceneJson;
    sceneJson["SceneName"] = "Untitled";

    json projectConfiguration;
    projectConfiguration["ProjectFolder"] = config.m_ProjectFolder;
    projectConfiguration["WindowTitle"] = config.windowTitle;
    projectConfiguration["WindowWidth"] = config.windowWidth;
    projectConfiguration["WindowHeight"] = config.windowHeight;
    projectConfiguration["VirtualWidth"] = config.virtualWidth;
    projectConfiguration["VirtualHeight"] = config.virtualHeight;
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

    if (!config.m_ProjectFolder.empty() && std::filesystem::exists(config.m_ProjectFolder)) {
        std::string inputFilePath = config.m_ProjectFolder + "/input.json";
        std::ofstream inputOut(inputFilePath);
        if (inputOut.is_open()) {
            inputOut << inputMapJson.dump(4);
            inputOut.close();
        }
    }

    json entitiesJson = json::array();

    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Engine::Entity entity = i;
        json entityJson;
        entityJson["EntityID"] = static_cast<int>(entity);
        if (registry.HasComponent<Engine::Name>(entity)) {
            entityJson["Name"] = registry.GetComponent<Engine::Name>(entity).value;
        }

        ComponentSerializer::SerializeAll(entity, registry, entityJson);

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
                                  std::optional<std::reference_wrapper<SceneConfig>> config) {
    std::ifstream fin(filepath);
    if (!fin.is_open()) {
        log_error("Failed to open file for reading: " + filepath);
        return false;
    }

    json sceneJson;
    fin >> sceneJson;

    log_info("Loaded scene: " + sceneJson.value("SceneName", "Unknown"));

    if (config.has_value() && sceneJson.contains("Configuration")) {
        auto projectConfiguration = sceneJson["Configuration"];
        if (projectConfiguration.contains("ProjectFolder")) {
            config->get().m_ProjectFolder = projectConfiguration["ProjectFolder"];
        }
        if (projectConfiguration.contains("WindowTitle")) {
            config->get().windowTitle = projectConfiguration["WindowTitle"];
        }
        if (projectConfiguration.contains("WindowWidth")) {
            config->get().windowWidth = projectConfiguration["WindowWidth"];
        }
        if (projectConfiguration.contains("WindowHeight")) {
            config->get().windowHeight = projectConfiguration["WindowHeight"];
        }
        if (projectConfiguration.contains("VirtualWidth")) {
            config->get().virtualWidth = projectConfiguration["VirtualWidth"];
        }
        if (projectConfiguration.contains("VirtualHeight")) {
            config->get().virtualHeight = projectConfiguration["VirtualHeight"];
        }
        if (projectConfiguration.contains("TargetFPS")) {
            config->get().targetFPS = projectConfiguration["TargetFPS"];
        }
        if (projectConfiguration.contains("BuildExecutableName")) {
            config->get().m_BuildExecutableName = projectConfiguration["BuildExecutableName"];
        }
        if (projectConfiguration.contains("BuildWindowTitle")) {
            config->get().m_BuildWindowTitle = projectConfiguration["BuildWindowTitle"];
        }
    }

    if (config.has_value()) {
        if (!registry.HasResource<ProjectContext>()) {
            registry.AddResource<ProjectContext>(config->get().m_ProjectFolder);
        } else {
            registry.GetResource<ProjectContext>().projectFolder = config->get().m_ProjectFolder;
        }

        std::string inputFilePath = config->get().m_ProjectFolder + "/input.json";
        std::ifstream inputIn(inputFilePath);
        if (inputIn.is_open()) {
            json inputMapJson;
            inputIn >> inputMapJson;
            config->get().m_InputMap.clear();
            for (auto it = inputMapJson.begin(); it != inputMapJson.end(); ++it) {
                std::vector<Engine::Keybind> keybinds;
                for (const auto& kbJson : it.value()) {
                    Engine::Keybind kb;
                    kb.key = kbJson.value("key", 0);
                    kb.composeKey = kbJson.value("composeKey", 0);
                    kb.type = static_cast<Engine::DeviceType>(kbJson.value("type", 0));
                    keybinds.push_back(kb);
                }
                config->get().m_InputMap[it.key()] = keybinds;
            }
        }
    }

    if (sceneJson.contains("Entities")) {
        for (const auto& entityJson : sceneJson["Entities"]) {
            Engine::Entity entity = registry.CreateEntity();

            if (entityJson.contains("Name") && entityJson["Name"].is_string()) {
                registry.AddComponent<Engine::Name>(
                    entity, Engine::Name(entityJson["Name"].get<std::string>()));
            }

            ComponentSerializer::DeserializeAll(entity, registry, entityJson);
        }
    }

    return true;
}

} // namespace Serialization
