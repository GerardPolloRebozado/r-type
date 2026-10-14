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
 * Writes @c test.scene, @c assets/, and @c input.json. Does not write @c main.cpp
 * and does not open a window. Refuses to overwrite an existing @c test.scene.
 * @param request Name, directory, and description.
 * @return Result with @c scenePath set when the project was created.
 */
ScaffoldResult CreateProject(const ProjectRequest& request);

/**
 * @brief Makes sure a project folder has a @c test.scene the editor can open.
 *
 * Writes an empty scene when the file is missing. Leaves an existing scene unchanged
 * and does not add the folder to the Gallery list.
 * @param request Name and directory of the project. Description is ignored.
 * @return Result with @c scenePath set to @c test.scene.
 */
ScaffoldResult EnsureProjectScene(const ProjectRequest& request);

/**
 * @brief Writes a system under the engine tree and lists it in Engine CMake.
 *
 * @param name C++ identifier. @c System is appended when it is not already the suffix.
 * @return Result. Does not open a window and does not register the system.
 */
ScaffoldResult CreateSystem(const std::string& name);

/**
 * @brief Writes a component, system, and plugin and lists them in Engine CMake.
 *
 * @param name C++ identifier. @c Plugin is appended when it is not already the suffix.
 * @return Result. Does not open a window.
 */
ScaffoldResult CreatePlugin(const std::string& name);

} // namespace Editor

#endif // EDITOR_SCAFFOLD_HPP_
