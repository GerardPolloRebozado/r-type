#ifndef EDITOR_EDITORCONFIG_HPP_
#define EDITOR_EDITORCONFIG_HPP_

/**
 * @file EditorConfig.hpp
 * @brief Configuration and project-context helpers for the Editor.
 */

#include "PathUtils.hpp"
#include "SceneConfig.hpp"
#include <string>

namespace Editor {

using Serialization::ProjectContext;
using Serialization::ResolveAssetPath;
using Serialization::ToProjectRelativePath;

/**
 * @struct EditorConfig
 * @brief Scene configuration extended with editor-only session state.
 *
 * It is used as the project configuration for the currently edited project and
 * also serialized to `.scene` files through the @ref Serialization::SceneConfig base.
 */
struct EditorConfig : public Serialization::SceneConfig {
    /** @brief Asset path selected in the Assets panel. */
    std::string m_SelectedAssetPath;
};

} // namespace Editor

#endif // EDITOR_EDITORCONFIG_HPP_