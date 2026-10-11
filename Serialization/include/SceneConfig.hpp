#ifndef SERIALIZATION_SCENECONFIG_HPP_
#define SERIALIZATION_SCENECONFIG_HPP_

#include "Core/Application.hpp"
#include "ECS/Registry.hpp"
#include "Inputs/IController.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace Serialization {

struct SceneConfig : public Engine::ApplicationConfig {
    std::string m_ProjectFolder = ".";
    std::string m_BuildExecutableName = "Game";
    std::string m_BuildWindowTitle = "Game";
    std::string m_InitialScenePath;

    // Map of Action Name -> List of Keybinds
    std::unordered_map<std::string, std::vector<Engine::Keybind>> m_InputMap;
};

} // namespace Serialization

#endif // SERIALIZATION_SCENECONFIG_HPP_
