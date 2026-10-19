#ifndef ENGINE_ASSETS_PROJECTPATHS_HPP_
#define ENGINE_ASSETS_PROJECTPATHS_HPP_

#include "ECS/Registry.hpp"
#include <filesystem>
#include <string>

namespace Engine {

/**
 * @brief Per-registry project root used to resolve relative asset paths.
 *
 * The folder itself is machine-local state (it comes from the locally open
 * project, e.g. ~/.config/rtype/projects.json) and must never be trusted
 * from a committed file: scene/entity payloads store project-relative paths,
 * and this resource anchors them back to disk on the current machine.
 */
struct ProjectContext {
    std::string projectFolder;
    ProjectContext() = default;
    explicit ProjectContext(std::string folder) : projectFolder(std::move(folder)) {}
};

/**
 * @brief Resolves a stored (usually project-relative) path to a loadable one.
 * @return The absolute path unchanged when it exists, otherwise the path
 *         joined onto the project folder (existing or not, so callers can
 *         report the attempted location), otherwise the input unchanged.
 */
inline std::filesystem::path ResolveAssetPath(Registry& registry, const std::string& path) {
    if (path.empty()) {
        return path;
    }
    std::filesystem::path p(path);
    if (p.is_absolute() && std::filesystem::exists(p)) {
        return p;
    }
    if (registry.HasResource<ProjectContext>()) {
        const auto& proj = registry.GetResource<ProjectContext>().projectFolder;
        if (!proj.empty()) {
            std::filesystem::path full = std::filesystem::path(proj) / p;
            if (std::filesystem::exists(full)) {
                return full;
            }
            std::filesystem::path fromFilename = std::filesystem::path(proj) / p.filename();
            if (std::filesystem::exists(fromFilename)) {
                return fromFilename;
            }
            return full;
        }
    }
    if (std::filesystem::exists(p)) {
        return p;
    }
    return p;
}

/**
 * @brief Legacy alias kept for Windows-branch call sites.
 * @see ResolveAssetPath
 */
inline std::filesystem::path ResolveProjectPath(Registry& registry, const std::string& path) {
    return ResolveAssetPath(registry, path);
}

/**
 * @brief Converts an absolute (or foreign) path to project-relative form for
 *        storage in committed files. Paths outside the project are left as-is.
 */
inline std::string ToProjectRelativePath(Registry& registry, const std::string& path) {
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

} // namespace Engine

#endif // ENGINE_ASSETS_PROJECTPATHS_HPP_
