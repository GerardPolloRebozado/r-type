#ifndef ENGINE_REFLECTION_COMPONENTREFLECT_HPP_
#define ENGINE_REFLECTION_COMPONENTREFLECT_HPP_

// Glaze reflection metadata for Engine types that automatic reflection cannot
// cover: structs with non-serializable members (excluded here), structs with
// callbacks, and enums with string names.
// Every other Engine component is an aggregate and serializes with zero
// declarations (Position, Rotation, ColliderComponent, Parent, Name, Input,
// AudioSource, Keybind, Vector2, Color, ...).
//
// Automatic reflection reads member names from compiler-generated text, which
// can differ between compilers. Give a component explicit names (like
// ShapeComponent below) whenever an automatic name comes out wrong.

#include "Components/Animation.hpp"
#include "Components/AudioSource.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Input.hpp"
#include "Components/Name.hpp"
#include "Components/NetworkComponent.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "Components/SpriteComponent.hpp"
#include "Components/Text.hpp"
#include "Core/Application.hpp"
#include "Inputs/IController.hpp"
#include "Resources/WaveSchedule.hpp"
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
        &Engine::SpriteComponent::scaleX, "scaleY", &Engine::SpriteComponent::scaleY, "flipX",
        &Engine::SpriteComponent::flipX, "flipY", &Engine::SpriteComponent::flipY);
};

template <> struct glz::meta<Engine::Text> {
    // NB: the runtime `font` handle is intentionally excluded.
    static constexpr auto value =
        glz::object("text", &Engine::Text::text, "fontsize", &Engine::Text::fontsize, "spacing",
                    &Engine::Text::spacing, "color", &Engine::Text::color);
};

template <> struct glz::meta<Engine::ShapeComponent> {
    // Explicit names: with automatic reflection, `color = WHITE` (a raylib
    // macro) was saved as "color)" by some compilers and "color" by others,
    // so scenes saved on one OS failed to load on another.
    static constexpr auto value = glz::object(
        "type", &Engine::ShapeComponent::type, "width", &Engine::ShapeComponent::width, "height",
        &Engine::ShapeComponent::height, "radius", &Engine::ShapeComponent::radius, "sides",
        &Engine::ShapeComponent::sides, "endOffset", &Engine::ShapeComponent::endOffset,
        "thickness", &Engine::ShapeComponent::thickness, "filled", &Engine::ShapeComponent::filled,
        "color", &Engine::ShapeComponent::color);
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

template <> struct glz::meta<Engine::NetworkComponent> {
    // NB: `networkId` is engine-assigned at runtime and intentionally excluded.
    static constexpr auto value = glz::object(
        "authority", &Engine::NetworkComponent::authority, "syncTransform",
        &Engine::NetworkComponent::syncTransform, "syncSprite",
        &Engine::NetworkComponent::syncSprite, "prefabPath", &Engine::NetworkComponent::prefabPath);
};

template <> struct glz::meta<raylib::Rectangle> {
    static constexpr auto value =
        glz::object("x", &raylib::Rectangle::x, "y", &raylib::Rectangle::y, "width",
                    &raylib::Rectangle::width, "height", &raylib::Rectangle::height);
};

template <> struct glz::meta<Engine::FrameData> {
    static constexpr auto value =
        glz::object("rect", &Engine::FrameData::rect, "pivotX", &Engine::FrameData::pivotX,
                    "pivotY", &Engine::FrameData::pivotY);
};

template <> struct glz::meta<Engine::AnimationClip> {
    static constexpr auto value = glz::object(
        "name", &Engine::AnimationClip::name, "assetPaths", &Engine::AnimationClip::assetPaths,
        "frameWidth", &Engine::AnimationClip::frameWidth, "frameHeight",
        &Engine::AnimationClip::frameHeight, "totalFrames", &Engine::AnimationClip::totalFrames,
        "frameData", &Engine::AnimationClip::frameData, "frameDuration",
        &Engine::AnimationClip::frameDuration, "loop", &Engine::AnimationClip::loop);
};

template <> struct glz::meta<Engine::Animation> {
    static constexpr auto value = glz::object(
        "clips", &Engine::Animation::clips, "currentAnimation",
        &Engine::Animation::currentAnimation, "frameDuration", &Engine::Animation::timer, "offsetX",
        &Engine::Animation::offsetX, "offsetY", &Engine::Animation::offsetY, "scaleX",
        &Engine::Animation::scaleX, "scaleY", &Engine::Animation::scaleY, "flipX",
        &Engine::Animation::flipX, "flipY", &Engine::Animation::flipY);
};

template <> struct glz::meta<Engine::WavePatternType> {
    static constexpr auto value =
        enumerate("line", Engine::WavePatternType::Line, "vformation",
                  Engine::WavePatternType::VFormation, "sine", Engine::WavePatternType::Sine,
                  "grid", Engine::WavePatternType::Grid, "custom", Engine::WavePatternType::Custom);
};

#endif // ENGINE_REFLECTION_COMPONENTREFLECT_HPP_