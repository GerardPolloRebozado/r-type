#ifndef EDITOR_GALLERY_HPP_
#define EDITOR_GALLERY_HPP_

/**
 * @file Gallery.hpp
 * @brief Project gallery UI, project metadata and font loading.
 */

#include <string>
#include <vector>
using std::string;

namespace Editor {

/**
 * @struct ProjectData
 * @brief Metadata describing one local engine project.
 */
struct ProjectData {
    /** @brief Human friendly project name. */
    std::string name;

    /** @brief User supplied description. */
    std::string description;

    /** @brief Last opened timestamp as displayed in the gallery. */
    std::string lastOpened;

    /** @brief Absolute path to the project folder. */
    std::string path;

    /** @brief Version identifier of the engine the project targets. */
    std::string engineVersion;
};

/**
 * @class Gallery
 * @brief Startup project browser and project list persistence.
 */
class Gallery {
  public:
    /**
     * @brief Draws the project gallery window.
     * @return Path to the selected project, or an empty string when none.
     */
    std::string DrawGallery();

    /**
     * @brief Finds every project registered on this machine.
     * @return Collection of project metadata entries.
     */
    static std::vector<ProjectData> FindProjects();

    /**
     * @brief Adds or refreshes the gallery entry for a project path.
     * @param projectPath Absolute project path.
     */
    static void UpdateProjectList(const std::string& projectPath);
};
}; // namespace Editor

/**
 * @brief Loads the icon/typography fonts required by the gallery.
 */
void LoadGalleryFonts();

#endif