// NOTE: this TU must NOT include any Engine/raylib header.
// portable-file-dialogs requires the full windows.h (winuser.h), which
// clashes with raylib's CloseWindow/ShowCursor/DrawText/Rectangle names.
#include "FileDialog.hpp"

#include "portable-file-dialogs.h"

namespace Editor {
std::string PickFolderDialog(const std::string& title, const std::string& defaultPath) {
    const std::string base = defaultPath.empty() ? pfd::path::home() : defaultPath;
    auto dir = pfd::select_folder(title, base).result();
    return dir;
}
} // namespace Editor
