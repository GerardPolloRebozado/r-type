#ifndef EDITOR_EDITORCONFIG_HPP_
#define EDITOR_EDITORCONFIG_HPP_

#include "PathUtils.hpp"
#include "SceneConfig.hpp"
#include <string>

namespace Editor {

using Serialization::ProjectContext;
using Serialization::ResolveAssetPath;
using Serialization::ToProjectRelativePath;

struct EditorConfig : public Serialization::SceneConfig {
    std::string m_SelectedAssetPath;

    /**
     * @brief Absolute path of the scene open in the editor.
     *
     * Save, Build and Play write to this file. Editor-only: the serializer
     * takes a SceneConfig, so it is never stored in a scene file. Not the
     * same as m_InitialScenePath, the scene the game starts with (absolute,
     * mirrors "startupScene" in the project's project.json).
     */
    std::string m_ActiveScenePath;

    /**
     * @brief Scene a panel asked to open (absolute path), or empty.
     *
     * Panels only set it. EditorLayer handles it at the start of the next
     * frame (asking to save unsaved changes first) and clears it.
     */
    std::string m_RequestedScenePath;

    /**
     * @brief Folder a panel asked to create a new scene in, or empty.
     * EditorLayer asks for the name, then clears it.
     */
    std::string m_RequestedNewSceneFolder;

    /**
     * @brief Scene a panel asked to make the startup scene (absolute path), or
     * empty. EditorLayer writes project.json and updates m_InitialScenePath.
     */
    std::string m_RequestedStartupScene;
};

} // namespace Editor

#endif // EDITOR_EDITORCONFIG_HPP_
