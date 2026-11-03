#ifndef UI_UICOMPONENTREFLECT_HPP_
#define UI_UICOMPONENTREFLECT_HPP_

#include "UI/UiComponent.hpp"
#include <glaze/glaze.hpp>

template <> struct glz::meta<Engine::UiComponent> {
    // loaded and scriptTable stay out of the file, same as ScriptComponent.
    static constexpr auto value = glz::object("scriptPath", &Engine::UiComponent::scriptPath,
                                              "autoReload", &Engine::UiComponent::autoReload);
};

#endif // UI_UICOMPONENTREFLECT_HPP_
