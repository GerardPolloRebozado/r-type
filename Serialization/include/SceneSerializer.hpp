#ifndef SERIALIZATION_SCENESERIALIZER_HPP_
#define SERIALIZATION_SCENESERIALIZER_HPP_

#include "ECS/Registry.hpp"
#include "GlazeSerializer.hpp"
#include "SceneConfig.hpp"
#include <functional>
#include <optional>
#include <string>

#include "../../Editor/include/Panels/HierarchyPanel.hpp"
namespace Serialization {

struct SceneData {
    std::string sceneName;
    SceneConfig config;
    GlazeSerializer::SceneSnapshot entities;
    Editor::EntityDirectoriesManager directories;
};

class SceneSerializer {
  public:
    SceneSerializer() = default;

    void Serialize(const std::string& filepath, Engine::Registry& registry,
                   const SceneConfig& config,
                   SerializationFormat format = SerializationFormat::JSON);

    bool Deserialize(const std::string& filepath, Engine::Registry& registry,
                     std::optional<std::reference_wrapper<SceneConfig>> config = std::nullopt,
                     SerializationFormat format = SerializationFormat::JSON);

    void SetFormat(SerializationFormat format);
    SerializationFormat GetFormat() const;

  private:
    GlazeSerializer m_Serializer;
};

} // namespace Serialization

#endif // SERIALIZATION_SCENESERIALIZER_HPP_