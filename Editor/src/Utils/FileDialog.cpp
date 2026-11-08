// NOTE: this TU must NOT include any Engine/raylib header.
// portable-file-dialogs requires the full windows.h (winuser.h), which
// clashes with raylib's CloseWindow/ShowCursor/DrawText/Rectangle names.
#include "FileDialog.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#include "portable-file-dialogs.h"

namespace Editor {
std::string PickFolderDialog(const std::string& title, const std::string& defaultPath) {
    const std::string base = defaultPath.empty() ? pfd::path::home() : defaultPath;
    auto dir = pfd::select_folder(title, base).result();
    return dir;
}

std::vector<std::string> PickFilesDialog(const std::string& title, const std::string& defaultPath) {
    const std::string base = defaultPath.empty() ? pfd::path::home() : defaultPath;

    auto files = pfd::open_file(title, base, {"Image Files", "*.png *.jpg *.jpeg *.bmp"},
                                pfd::opt::multiselect)
                     .result();

    return files;
}

} // namespace Editor
