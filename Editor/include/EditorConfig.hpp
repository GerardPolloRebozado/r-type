#ifndef EDITOR_EDITORCONFIG_HPP_
#define EDITOR_EDITORCONFIG_HPP_

#include "Core/Application.hpp"
#include <string>

namespace Editor {

struct EditorConfig : public Engine::ApplicationConfig {
    std::string m_ProjectFolder;
    std::string m_SelectedAssetPath;
    std::string m_BuildExecutableName = "Game";
    std::string m_BuildWindowTitle = "Game";
};

} // namespace Editor

#endif // EDITOR_EDITORCONFIG_HPP_
