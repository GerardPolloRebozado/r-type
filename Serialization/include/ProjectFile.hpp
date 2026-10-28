#ifndef SERIALIZATION_PROJECTFILE_HPP_
#define SERIALIZATION_PROJECTFILE_HPP_

#include <filesystem>
#include <string>

namespace Serialization {

/** @brief Name of the project file, at the root of every project folder. */
inline constexpr const char* kProjectFileName = "project.json";

/** @brief Scene used when a project has no project.json (older projects). */
inline constexpr const char* kDefaultStartupScene = "test.scene";

/**
 * @struct ProjectFile
 * @brief Settings shared by the whole project, stored in @c project.json.
 *
 * Unlike the settings inside each scene file, there is one copy per project.
 * Unknown keys are ignored when reading, so fields can be added later without
 * breaking older editors.
 *
 * @code{.json}
 * {"startupScene": "levels/Menu.scene"}
 * @endcode
 */
struct ProjectFile {
    /** @brief Scene the game starts with, relative to the project folder. */
    std::string startupScene = kDefaultStartupScene;
};

/**
 * @brief Reads @c project.json from @p folder into @p out.
 * @return False if the file is missing or can't be read; @p out then keeps
 *         its default values.
 */
bool LoadProjectFile(const std::filesystem::path& folder, ProjectFile& out);

/** @brief Writes @p project to @c project.json in @p folder. */
bool SaveProjectFile(const std::filesystem::path& folder, const ProjectFile& project);

/**
 * @brief Absolute path of the project's startup scene.
 *
 * The scene named in @c project.json if that file exists, otherwise
 * @c test.scene in @p folder (which may not exist yet).
 */
std::filesystem::path StartupScenePath(const std::filesystem::path& folder);

} // namespace Serialization

#endif // SERIALIZATION_PROJECTFILE_HPP_
