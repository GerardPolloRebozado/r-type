#include "ProjectFile.hpp"
#include "Logging/Logger.hpp"
#include <fstream>
#include <glaze/glaze.hpp>
#include <iterator>

// Explicit key names: the same on every compiler and OS.
template <> struct glz::meta<Serialization::ProjectFile> {
    static constexpr auto value =
        glz::object("startupScene", &Serialization::ProjectFile::startupScene);
};

namespace Serialization {

bool LoadProjectFile(const std::filesystem::path& folder, ProjectFile& out) {
    const std::filesystem::path path = folder / kProjectFileName;
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    const std::string content((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    ProjectFile project;
    constexpr glz::opts options{.error_on_unknown_keys = false};
    if (const auto error = glz::read<options>(project, content)) {
        log_error("Failed to read " + path.string() + ": " + glz::format_error(error, content));
        return false;
    }
    if (project.startupScene.empty()) {
        project.startupScene = kDefaultStartupScene;
    }
    out = std::move(project);
    return true;
}

bool SaveProjectFile(const std::filesystem::path& folder, const ProjectFile& project) {
    const std::filesystem::path path = folder / kProjectFileName;
    std::string json;
    if (glz::write<glz::opts{.prettify = true}>(project, json)) {
        log_error("Failed to encode " + path.string());
        return false;
    }
    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open()) {
        log_error("Failed to write " + path.string());
        return false;
    }
    file << json << "\n";
    return file.good();
}

std::filesystem::path StartupScenePath(const std::filesystem::path& folder) {
    ProjectFile project;
    LoadProjectFile(folder, project);
    std::error_code error;
    const std::filesystem::path startup = (folder / project.startupScene).lexically_normal();
    if (std::filesystem::is_regular_file(startup, error)) {
        return startup;
    }
    if (project.startupScene != kDefaultStartupScene) {
        log_error("Startup scene not found: " + startup.string() + ". Using " +
                  kDefaultStartupScene + ".");
    }
    return (folder / kDefaultStartupScene).lexically_normal();
}

} // namespace Serialization
