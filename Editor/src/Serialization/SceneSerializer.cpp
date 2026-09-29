#include "Serialization/SceneSerializer.hpp"
#include "TransformEditor.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Editor {

void SceneSerializer::Serialize(const std::string& filepath, Engine::Registry& registry) {
    json sceneJson;
    sceneJson["SceneName"] = "Untitled";

    json entitiesJson = json::array();

    // We instantiate the editors here (later they will be centralized)
    TransformEditor transformEditor;

    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Engine::Entity entity = static_cast<Engine::Entity>(i);
        json entityJson;
        entityJson["EntityID"] = (int)entity;

        transformEditor.Serialize(entity, registry, entityJson);

        entitiesJson.push_back(entityJson);
    }

    sceneJson["Entities"] = entitiesJson;

    std::ofstream fout(filepath);
    if (fout.is_open()) {
        fout << sceneJson.dump(4);
        fout.close();
        std::cout << "Successfully saved scene to " << filepath << std::endl;
    } else {
        std::cerr << "Failed to open file for writing: " << filepath << std::endl;
    }
}

bool SceneSerializer::Deserialize(const std::string& filepath, Engine::Registry& registry) {
    std::ifstream fin(filepath);
    if (!fin.is_open()) {
        std::cerr << "Failed to open file for reading: " << filepath << std::endl;
        return false;
    }

    json sceneJson;
    fin >> sceneJson;

    std::cout << "Loaded scene: " << sceneJson.value("SceneName", "Unknown") << std::endl;

    TransformEditor transformEditor;

    if (sceneJson.contains("Entities")) {
        for (const auto& entityJson : sceneJson["Entities"]) {
            Engine::Entity entity = registry.CreateEntity();

            // Deserialize components back onto the newly spawned entity
            transformEditor.Deserialize(entity, registry, entityJson);
        }
    }

    return true;
}

} // namespace Editor
