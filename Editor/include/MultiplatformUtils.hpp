#ifndef MULTIPLATFORM_UTILS_HPP_
#define MULTIPLATFORM_UTILS_HPP_

/**
 * @file MultiplatformUtils.hpp
 * @brief Small cross-platform helpers used by the editor.
 */

#include <filesystem>

namespace Editor {

/**
 * @brief Returns the user configuration directory for the editor.
 * @return Configuration directory path.
 */
std::filesystem::path ConfigDirectory();

/**
 * @brief Formats a raw timestamp into a human readable date.
 * @param timestamp Timestamp string to format.
 * @return Formatted date string.
 */
std::string FormatDate(const std::string& timestamp);

} // namespace Editor

#endif // MULTIPLATFORM_UTILS_HPP_