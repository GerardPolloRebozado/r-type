#ifndef UI_UICOMPONENTREFLECT_HPP_
#define UI_UICOMPONENTREFLECT_HPP_

#include "UI/UiComponent.hpp"
#include <glaze/glaze.hpp>

template <> struct glz::meta<Engine::UiValueKind> {
    static constexpr auto value = glz::enumerate(
        "bool", Engine::UiValueKind::Bool, "number", Engine::UiValueKind::Number, "string",
        Engine::UiValueKind::String, "stringList", Engine::UiValueKind::StringList, "color",
        Engine::UiValueKind::Color);
};

template <> struct glz::meta<Engine::UiProp> {
    static constexpr auto value = glz::object(
        "key", &Engine::UiProp::key, "kind", &Engine::UiProp::kind, "boolValue",
        &Engine::UiProp::boolValue, "numberValue", &Engine::UiProp::numberValue, "stringValue",
        &Engine::UiProp::stringValue, "stringList", &Engine::UiProp::stringList, "colorSet",
        &Engine::UiProp::colorSet, "colorValue", &Engine::UiProp::colorValue);
};

template <> struct glz::meta<Engine::UiActionType> {
    static constexpr auto value = glz::enumerate(
        "none", Engine::UiActionType::None, "goToPage", Engine::UiActionType::GoToPage, "back",
        Engine::UiActionType::Back, "callMethod", Engine::UiActionType::CallMethod, "loadScene",
        Engine::UiActionType::LoadScene, "quit", Engine::UiActionType::Quit);
};

template <> struct glz::meta<Engine::UiAction> {
    static constexpr auto value =
        glz::object("type", &Engine::UiAction::type, "target", &Engine::UiAction::target);
};

template <> struct glz::meta<Engine::UiComponent> {
    // loaded and scriptTable stay out of the file, same as ScriptComponent.
    static constexpr auto value = glz::object(
        "scriptPath", &Engine::UiComponent::scriptPath, "autoReload",
        &Engine::UiComponent::autoReload, "template", &Engine::UiComponent::templateName, "props",
        &Engine::UiComponent::props, "page", &Engine::UiComponent::page, "startPage",
        &Engine::UiComponent::startPage, "showOn", &Engine::UiComponent::showOn, "order",
        &Engine::UiComponent::order, "onClick", &Engine::UiComponent::onClick, "onCancel",
        &Engine::UiComponent::onCancel);
};

#endif // UI_UICOMPONENTREFLECT_HPP_
