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
     * same as m_InitialScenePath, the scene the game starts with.
     */
    std::string m_ActiveScenePath;
};

} // namespace Editor

#endif // EDITOR_EDITORCONFIG_HPP_
