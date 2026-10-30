/**
 * @file Scaffold.hpp
 * @brief Creates editor projects and engine system or plugin source files.
 *
 * The @c RTypeEditor @c new and @c create commands call these functions.
 * A later Gallery button should call them too, so file generation stays in one place.
 */

#ifndef EDITOR_SCAFFOLD_HPP_
#define EDITOR_SCAFFOLD_HPP_

#include <string>

namespace Editor {

/**
 * @brief Inputs for @ref CreateProject.
 */
struct ProjectRequest {
    /** @brief Display name. Also used as the window title. */
    std::string name;
    /** @brief Directory to create. Empty means ./<name> in the working directory. */
    std::string path;
    /** @brief Optional text stored in the Gallery project list. */
    std::string description;
};

/**
 * @brief Outcome of a scaffold command.
 *
 * @c ok is false when nothing was created. @c message is the same text written
 * to the log. @c scenePath is set by @ref CreateProject to the new @c test.scene.
 */
struct ScaffoldResult {
    bool ok;
    std::string message;
    std::string scenePath;
};

/**
 * @brief Creates an editor project folder and registers it in the Gallery list.
 *
 * Writes @c test.scene, @c project.json (startup scene: @c test.scene),
 * @c assets/, and @c input.json. Does not write @c main.cpp and does not open
 * a window. Refuses to overwrite an existing @c test.scene.
 * @param request Name, directory, and description.
 * @return Result with @c scenePath set when the project was created.
 */
ScaffoldResult CreateProject(const ProjectRequest& request);

/**
 * @brief Finds the scene the editor opens for a project folder.
 *
 * That is the startup scene named in @c project.json. If it is missing, falls
 * back to @c test.scene, writing an empty one when needed. Projects without a
 * @c project.json get one. Leaves existing scenes unchanged and does not add
 * the folder to the Gallery list.
 * @param request Name and directory of the project. Description is ignored.
 * @return Result with @c scenePath set to the scene to open.
 */
ScaffoldResult EnsureProjectScene(const ProjectRequest& request);

/**
 * @brief Writes a system in the working tree and updates CMake so it builds.
 *
 * @param name C++ identifier. @c System is appended when it is not already the suffix.
 * @param outputPath Directory where files are written. Empty uses the current working directory.
 *     When that folder is the engine (or inside it), files go under @c include/ and @c src/ and
 *     @c Engine/CMakeLists.txt is updated. Otherwise files go under @c system/ in the project
 *     and @c system/build.cmake is updated for editor builds.
 * @return Result. Does not open a window.
 */
ScaffoldResult CreateSystem(const std::string& name, const std::string& outputPath = "");

/**
 * @brief Writes a component, system, and plugin and updates CMake so they build.
 *
 * @param name C++ identifier. @c Plugin is appended when it is not already the suffix.
 * @param outputPath Same rules as @ref CreateSystem.
 * @return Result. Does not open a window.
 */
ScaffoldResult CreatePlugin(const std::string& name, const std::string& outputPath = "");

} // namespace Editor

#endif // EDITOR_SCAFFOLD_HPP_
