#ifndef EDITOR_FILEDIALOG_HPP_
#define EDITOR_FILEDIALOG_HPP_

#include <string>
#include <vector>
namespace Editor {
// Opens a native folder-picker dialog.
// Isolated in its own translation unit because portable-file-dialogs needs
// the FULL windows.h (winuser.h: MessageBoxW, IDOK, ...), which collides
// with raylib (CloseWindow, ShowCursor, DrawText, Rectangle, ...).
// No Engine/raylib header may be included from FileDialog.cpp.
std::string PickFolderDialog(const std::string& title, const std::string& defaultPath = "");
std::vector<std::string> PickFilesDialog(const std::string& title,
                                         const std::string& defaultPath = "");
} // namespace Editor

#endif
