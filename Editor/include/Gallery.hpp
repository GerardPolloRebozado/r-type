#ifndef EDITOR_GALLERY_HPP_
#define EDITOR_GALLERY_HPP_

#include <string>
#include <vector>
using std::string;

namespace Editor {
struct ProjectData {
    std::string name;
    std::string description;
    std::string lastOpened;
    std::string path;
    std::string engineVersion;
};

class Gallery {
  public:
    std::string DrawGallery();
    static std::vector<ProjectData> FindProjects();
    static void UpdateProjectList(const std::string& projectPath);
};
}; // namespace Editor

void LoadGalleryFonts();

#endif
