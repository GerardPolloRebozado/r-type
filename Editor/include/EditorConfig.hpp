#ifndef EDITOR_EDITORCONFIG_HPP_
#define EDITOR_EDITORCONFIG_HPP_

#include "Core/Application.hpp"
#include "Inputs/IController.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace Editor {

struct EditorConfig : public Engine::ApplicationConfig {
    std::string m_ProjectFolder;
    std::string m_SelectedAssetPath;
    std::string m_BuildExecutableName = "Game";
    std::string m_BuildWindowTitle = "Game";
    std::string m_InitialScenePath;

    // Map of Action Name -> List of Keybinds
    Engine::IController::Kmap m_InputMap;
};

} // namespace Editor

#endif // EDITOR_EDITORCONFIG_HPP_
