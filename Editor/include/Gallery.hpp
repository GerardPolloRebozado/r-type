#ifndef EDITOR_GALLERY_HPP_
#define EDITOR_GALLERY_HPP_

#include <string>
#include <vector>
using std::string;

namespace Editor {

/**
 * @struct ProjectData
 * @brief One row of the Gallery project list stored in @c projects.json.
 *
 * Field names are the JSON keys. @ref GalleryReflect maps them for Glaze.
 */
struct ProjectData {
    /** @brief Name shown in the Gallery. */
    std::string name;
    /** @brief Optional description shown with the project. */
    std::string description;
    /** @brief Unix time in seconds, stored as text. */
    std::string lastOpened;
    /** @brief Absolute path of the project folder. */
    std::string path;
    /** @brief Engine version recorded when the project was created. */
    std::string engineVersion;
};

/**
 * @class Gallery
 * @brief Project picker shown before the editor opens a scene.
 */
class Gallery {
  public:
    /**
     * @brief Draws the Gallery and returns the scene to open.
     * @return Path of @c test.scene when a project is chosen, otherwise empty.
     */
    std::string DrawGallery();

    /**
     * @brief Reads the Gallery project list from @c projects.json.
     * @return Parsed projects. Empty when the file is missing or invalid.
     */
    static std::vector<ProjectData> FindProjects();

    /**
     * @brief Stores the current time as @c lastOpened for one project path.
     * @param projectPath Absolute path stored in @ref ProjectData::path.
     */
    static void UpdateProjectList(const std::string& projectPath);
};
}; // namespace Editor

/**
 * @brief Loads the fonts used by the Gallery window.
 */
void LoadGalleryFonts();

#endif
