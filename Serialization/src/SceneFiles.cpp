#include "SceneFiles.hpp"
#include "SceneSerializer.hpp"
#include <optional>

namespace Serialization {

Engine::SceneManager& EnableSceneFiles(Engine::Registry& registry) {
    if (!registry.HasResource<Engine::SceneManager>()) {
        registry.AddResource<Engine::SceneManager>();
    }
    auto& scenes = registry.GetResource<Engine::SceneManager>();
    scenes.SetFileLoader([](Engine::Registry& reg, const std::string& path) {
        SceneSerializer serializer;
        return serializer.Deserialize(path, reg, std::nullopt);
    });
    return scenes;
}

} // namespace Serialization
