/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** Project folder context + asset path resolution (Engine side).
**
** Games and the editor run with the executable's directory as CWD, while
** asset/script paths stored in scenes are project-relative
** (e.g. "scripts/test.lua"). This resolver turns them into loadable paths
** using the ProjectContext resource (registered by the editor and by
** SceneSerializer::Deserialize).
*/
#ifndef ENGINE_RESOURCES_PROJECTCONTEXT_HPP_
#define ENGINE_RESOURCES_PROJECTCONTEXT_HPP_

#include "ECS/Registry.hpp"

#include <filesystem>
#include <string>

namespace Engine {

struct ProjectContext {
    std::string projectFolder;
    ProjectContext() = default;
    explicit ProjectContext(std::string folder) : projectFolder(std::move(folder)) {}
};

/**
 * @brief Resolves a stored (possibly project-relative) path to a loadable one.
 * Absolute paths that exist are returned unchanged. Otherwise the path is
 * looked up under the ProjectContext folder (plus a filename fallback).
 * The best candidate is always returned, even if it does not exist, so
 * error messages point at the right location.
 */
inline std::filesystem::path ResolveProjectPath(Registry& registry, const std::string& path) {
    if (path.empty()) {
        return std::filesystem::path{};
    }
    std::filesystem::path p(path);
    std::error_code ec;
    if (p.is_absolute() && std::filesystem::exists(p, ec)) {
        return p;
    }
    if (registry.HasResource<ProjectContext>()) {
        const auto& proj = registry.GetResource<ProjectContext>().projectFolder;
        if (!proj.empty()) {
            std::filesystem::path full = std::filesystem::path(proj) / p;
            if (std::filesystem::exists(full, ec)) {
                return full;
            }
            std::filesystem::path fromFilename = std::filesystem::path(proj) / p.filename();
            if (std::filesystem::exists(fromFilename, ec)) {
                return fromFilename;
            }
            return full;
        }
    }
    return p;
}

} // namespace Engine

#endif // ENGINE_RESOURCES_PROJECTCONTEXT_HPP_
