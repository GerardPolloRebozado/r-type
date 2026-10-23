#include "SceneFiles.hpp"
#include "SceneSerializer.hpp"
#include <optional>
#include <string_view>

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

std::string PickStartupScene(const std::vector<std::string>& args, const std::string& fallback) {
    constexpr std::string_view extension = ".scene";
    for (const auto& arg : args) {
        if (arg.size() > extension.size() && arg.ends_with(extension) && !arg.starts_with("-")) {
            return arg;
        }
    }
    return fallback;
}

} // namespace Serialization
