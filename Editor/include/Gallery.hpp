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
    void DrawGallery();

  private:
    std::vector<ProjectData> FindProjects();
};
}; // namespace Editor

void LoadGalleryFonts();

#endif
