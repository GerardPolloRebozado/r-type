/**
 * @file ProjectSystemsBuild.hpp
 * @brief CMake and registration glue for project-local scaffolded systems and plugins.
 */

#ifndef EDITOR_PROJECTSYSTEMSBUILD_HPP_
#define EDITOR_PROJECTSYSTEMSBUILD_HPP_

#include <filesystem>
#include <string>

namespace Editor {

/** @brief Subfolder under a Gallery project where CLI scaffold writes C++ sources. */
inline constexpr const char* kProjectSystemsDir = "system";

/** @brief Generated list of project system sources for editor builds. */
inline constexpr const char* kProjectBuildCmake = "system/build.cmake";

/** @brief Generated registration hook included from @c main.cpp. */
inline constexpr const char* kProjectRegisterHeader = "system/rtype_scaffold_register.hpp";

std::filesystem::path ProjectSystemsDirectory(const std::filesystem::path& projectRoot);

/**
 * @brief Adds a @c .cpp path under @c system/ to @c system/build.cmake.
 * @param projectRoot Gallery project directory (contains @c test.scene or @c main.cpp).
 * @param cppFileName Base name such as @c TestSystem.cpp.
 */
bool AppendProjectSystemSource(const std::filesystem::path& projectRoot,
                               const std::string& cppFileName, std::string& error);

/**
 * @brief Rewrites @ref kProjectRegisterHeader from files present in @c system/.
 */
bool RegenerateProjectRegister(const std::filesystem::path& projectRoot, std::string& error);

/** @brief Extra CMake lines for @c .editor_build.cmake when project systems exist. */
std::string ProjectSystemsEditorBuildSnippet(const std::string& projectFolder);

/** @brief Extra C++ lines for generated @c main.cpp when the register header exists. */
std::string ProjectSystemsMainCppSnippet(const std::string& projectFolder);

} // namespace Editor

#endif // EDITOR_PROJECTSYSTEMSBUILD_HPP_
