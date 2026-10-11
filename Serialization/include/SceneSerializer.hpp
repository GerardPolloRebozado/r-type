#ifndef SERIALIZATION_SCENESERIALIZER_HPP_
#define SERIALIZATION_SCENESERIALIZER_HPP_

#include "ECS/Registry.hpp"
#include "SceneConfig.hpp"
#include <functional>
#include <optional>
#include <string>

namespace Serialization {

/**
 * @brief Handles serialization of the active Scene/Registry to and from disk.
 *
 * Encapsulates saving and loading entity hierarchies, components, and project scene settings
 * without any dependency on the Editor or UI backends (ImGui).
 */
class SceneSerializer {
  public:
    SceneSerializer() = default;

    void Serialize(const std::string& filepath, Engine::Registry& registry,
                   const SceneConfig& config);
    bool Deserialize(const std::string& filepath, Engine::Registry& registry,
                     std::optional<std::reference_wrapper<SceneConfig>> config = std::nullopt);
};

} // namespace Serialization

#endif // SERIALIZATION_SCENESERIALIZER_HPP_
