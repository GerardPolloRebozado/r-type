#ifndef SERIALIZATION_GLAZE_REFLECT_HPP_
#define SERIALIZATION_GLAZE_REFLECT_HPP_

// Engine component reflection (shared with networking/game code via Engine).
#include "PathUtils.hpp"
#include "Reflection/ComponentReflect.hpp"
#include "SceneConfig.hpp"
#include <glaze/glaze.hpp>

template <> struct glz::meta<Serialization::SceneConfig> {
    static constexpr auto value =
        glz::object("windowWidth", &Serialization::SceneConfig::windowWidth, "windowHeight",
                    &Serialization::SceneConfig::windowHeight, "virtualWidth",
                    &Serialization::SceneConfig::virtualWidth, "virtualHeight",
                    &Serialization::SceneConfig::virtualHeight, "maximized",
                    &Serialization::SceneConfig::maximized, "windowTitle",
                    &Serialization::SceneConfig::windowTitle, "targetFPS",
                    &Serialization::SceneConfig::targetFPS, "projectFolder",
                    &Serialization::SceneConfig::m_ProjectFolder, "buildExecutableName",
                    &Serialization::SceneConfig::m_BuildExecutableName, "buildWindowTitle",
                    &Serialization::SceneConfig::m_BuildWindowTitle, "initialScenePath",
                    &Serialization::SceneConfig::m_InitialScenePath, "inputMap",
                    &Serialization::SceneConfig::m_InputMap);
};

template <> struct glz::meta<Serialization::ProjectContext> {
    static constexpr auto value =
        glz::object("projectFolder", &Serialization::ProjectContext::projectFolder);
};

#endif // SERIALIZATION_GLAZE_REFLECT_HPP_
