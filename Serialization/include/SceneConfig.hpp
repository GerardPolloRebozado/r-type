#ifndef SERIALIZATION_SCENECONFIG_HPP_
#define SERIALIZATION_SCENECONFIG_HPP_

#include "Core/Application.hpp"
#include "Inputs/IController.hpp"
#include "Resources/WaveSchedule.hpp"
#include <string>
#include <unordered_map>

namespace Serialization {

struct SceneConfig : public Engine::ApplicationConfig {
    std::string m_ProjectFolder = ".";
    std::string m_BuildExecutableName = "Game";
    std::string m_BuildWindowTitle = "Game";
    bool m_BuildDebugSymbols = false;
    bool m_BuildWasm = false;
    std::string m_InitialScenePath;

    /** @brief Enemy waves configured from the editor panel **/
    std::vector<Engine::WaveDefinition> m_Waves;

    int m_MultiplayerClientCount = 2;
    bool m_MultiplayerServerHeadless = false;

    // Map of Action Name -> shortcuts + whether the action is networked
    std::unordered_map<std::string, Engine::ActionBinding> m_InputMap;
};

} // namespace Serialization

#endif // SERIALIZATION_SCENECONFIG_HPP_