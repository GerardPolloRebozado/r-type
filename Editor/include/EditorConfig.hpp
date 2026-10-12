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
};

} // namespace Editor

#endif // EDITOR_EDITORCONFIG_HPP_
