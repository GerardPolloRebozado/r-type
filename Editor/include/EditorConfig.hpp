#ifndef EDITOR_EDITORCONFIG_HPP_
#define EDITOR_EDITORCONFIG_HPP_

#include "Core/Application.hpp"
#include "ECS/Registry.hpp"
#include "Inputs/IController.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Editor {

struct ProjectContext {
    std::string projectFolder;
};

inline std::filesystem::path ResolveAssetPath(Engine::Registry& registry, const std::string& path) {
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

struct EditorConfig : public Engine::ApplicationConfig {
    std::string m_ProjectFolder;
    std::string m_SelectedAssetPath;
    std::string m_BuildExecutableName = "Game";
    std::string m_BuildWindowTitle = "Game";
    std::string m_InitialScenePath;

    // Map of Action Name -> List of Keybinds
    Engine::IController::Kmap m_InputMap;
};

} // namespace Editor

#endif // EDITOR_EDITORCONFIG_HPP_
