#ifndef MULTIPLATFORM_UTILS_HPP_
#define MULTIPLATFORM_UTILS_HPP_

#include <filesystem>

namespace Editor {
std::filesystem::path ConfigDirectory();
std::string FormatDate(const std::string& timestamp);
} // namespace Editor

#endif
