#ifndef ENGINE_REFLECTION_COMPONENTREFLECT_HPP_
#define ENGINE_REFLECTION_COMPONENTREFLECT_HPP_

// Glaze reflection metadata for Engine types that automatic reflection cannot
// cover: structs with non-serializable members (excluded here), structs with
// callbacks, and enums with string names.
// Every other Engine component is an aggregate and serializes with zero
// declarations (Position, Rotation, ShapeComponent, ColliderComponent,
// Parent, Name, Input, AudioSource, Keybind, Vector2, Color, ...).

#include "Components/AudioSource.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Input.hpp"
#include "Components/Name.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "Components/SpriteComponent.hpp"
#include "Components/Text.hpp"
#include "Core/Application.hpp"
#include "Inputs/IController.hpp"
#include "Scripting/ScriptComponent.hpp"
#include <glaze/glaze.hpp>
#include <raylib.h>

namespace Engine {} // namespace Engine

template <> struct glz::meta<Engine::Position> {
    // NB: Position has a constructor (draw-layer z), so it is not an
    // aggregate and needs explicit reflection. Missing keys in old scenes
    // keep their default values (z/prevZ default to 0).
    static constexpr auto value =
        glz::object("x", &Engine::Position::x, "y", &Engine::Position::y, "z", &Engine::Position::z,
                    "prevX", &Engine::Position::prevX, "prevY", &Engine::Position::prevY, "prevZ",
                    &Engine::Position::prevZ, "moved", &Engine::Position::moved, "relative",
                    &Engine::Position::relative);
};

template <> struct glz::meta<Engine::SpriteComponent> {
    // NB: the runtime `sprite` pointer is intentionally excluded; only the
    // asset reference and transform are serialized.
    static constexpr auto value = glz::object(
        "assetPath", &Engine::SpriteComponent::assetPath, "offsetX",
        &Engine::SpriteComponent::offsetX, "offsetY", &Engine::SpriteComponent::offsetY, "scaleX",
        &Engine::SpriteComponent::scaleX, "scaleY", &Engine::SpriteComponent::scaleY);
};

template <> struct glz::meta<Engine::Text> {
    // NB: the runtime `font` handle is intentionally excluded.
    static constexpr auto value =
        glz::object("text", &Engine::Text::text, "fontsize", &Engine::Text::fontsize, "spacing",
                    &Engine::Text::spacing, "color", &Engine::Text::color);
};

template <> struct glz::meta<Engine::ApplicationConfig> {
    // NB: the `onUI` / `customRender` callbacks are intentionally excluded.
    static constexpr auto value =
        glz::object("windowWidth", &Engine::ApplicationConfig::windowWidth, "windowHeight",
                    &Engine::ApplicationConfig::windowHeight, "maximized",
                    &Engine::ApplicationConfig::maximized, "windowTitle",
                    &Engine::ApplicationConfig::windowTitle, "targetFPS",
                    &Engine::ApplicationConfig::targetFPS);
};

template <> struct glz::meta<Engine::DeviceType> {
    static constexpr auto value =
        glz::enumerate("KEYBOARD", Engine::DeviceType::KEYBOARD, "MOUSE", Engine::DeviceType::MOUSE,
                       "GAMEPAD", Engine::DeviceType::GAMEPAD, "TOUCH", Engine::DeviceType::TOUCH);
};

template <> struct glz::meta<Engine::ShapeType> {
    static constexpr auto value = glz::enumerate(
        "RECTANGLE", Engine::ShapeType::RECTANGLE, "CIRCLE", Engine::ShapeType::CIRCLE, "POLYGON",
        Engine::ShapeType::POLYGON, "LINE", Engine::ShapeType::LINE);
};

template <> struct glz::meta<Engine::ScriptComponent> {
    // NB: `loaded` and the runtime `scriptTable` are intentionally excluded.
    static constexpr auto value = glz::object("scriptPath", &Engine::ScriptComponent::scriptPath,
                                              "autoReload", &Engine::ScriptComponent::autoReload);
};

template <> struct glz::meta<Engine::CameraComponent> {
    static constexpr auto value = glz::object(
        "zoom", &Engine::CameraComponent::zoom, "primary", &Engine::CameraComponent::primary,
        "followTarget", &Engine::CameraComponent::followTarget, "followSpeed",
        &Engine::CameraComponent::followSpeed, "clampToBounds",
        &Engine::CameraComponent::clampToBounds, "bounds", &Engine::CameraComponent::bounds,
        "shakeTrauma", &Engine::CameraComponent::shakeTrauma, "shakeMagnitude",
        &Engine::CameraComponent::shakeMagnitude, "shakeDecay",
        &Engine::CameraComponent::shakeDecay, "shakeOffsetX",
        &Engine::CameraComponent::shakeOffsetX, "shakeOffsetY",
        &Engine::CameraComponent::shakeOffsetY);
};

#endif // ENGINE_REFLECTION_COMPONENTREFLECT_HPP_
