#ifndef SERIALIZATION_PATHUTILS_HPP_
#define SERIALIZATION_PATHUTILS_HPP_

#include "ECS/Registry.hpp"
#include "Resources/ProjectContext.hpp"
#include <filesystem>
#include <string>

namespace Serialization {

using ProjectContext = Engine::ProjectContext;

inline std::filesystem::path ResolveAssetPath(Engine::Registry& registry, const std::string& path) {
    return Engine::ResolveProjectPath(registry, path);
}

inline std::string ToProjectRelativePath(Engine::Registry& registry, const std::string& path) {
    if (path.empty()) {
        return path;
    }
    if (registry.HasResource<ProjectContext>()) {
        const auto& proj = registry.GetResource<ProjectContext>().projectFolder;
        if (!proj.empty()) {
            std::error_code ec;
            std::filesystem::path rel = std::filesystem::relative(path, proj, ec);
            if (!ec && !rel.empty() && rel.string().find("..") == std::string::npos) {
                return rel.string();
            }
            std::filesystem::path fromFilename =
                std::filesystem::path(proj) / std::filesystem::path(path).filename();
            if (std::filesystem::exists(fromFilename)) {
                return std::filesystem::path(path).filename().string();
            }
        }
    }
    return path;
}

} // namespace Serialization

#endif // SERIALIZATION_PATHUTILS_HPP_
