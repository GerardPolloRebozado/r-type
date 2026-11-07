#include "Scripting/LuaAPI/Plugins/AnimationAPI.hpp"
#include "Components/Animation.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>
#include <string>
#include <utility>
#include <vector>

namespace Engine::LuaAPI {
namespace {

Animation* FindAnimation(Registry& registry, Entity entity) {
    if (!registry.IsEntityAlive(entity) || !registry.HasComponent<Animation>(entity)) {
        return nullptr;
    }
    return &registry.RequireComponent<Animation>(entity);
}

bool ReadPaths(const sol::object& value, std::vector<std::string>& paths) {
    if (!value.is<sol::table>()) {
        return false;
    }
    for (const auto& [key, entry] : value.as<sol::table>()) {
        (void)key;
        if (entry.is<std::string>()) {
            paths.push_back(entry.as<std::string>());
        }
    }
    return !paths.empty();
}

} // namespace

void registerAnimation(sol::state_view lua, Registry& registry) {
    // Idempotent: Lua-first prototypes may never have run AnimationPlugin.
    registry.RegisterComponent<Animation>();

    sol::table component = Internal::ensureNamespace(lua, "Component");

    component["addAnimation"] =
        [&registry](Entity entity, sol::object pathsOrSheet, sol::optional<float> frameWidth,
                    sol::optional<float> frameHeight, sol::optional<int> totalFrames,
                    sol::optional<float> frameDuration, sol::optional<bool> loop) -> bool {
        if (!registry.IsEntityAlive(entity) || !pathsOrSheet.valid() ||
            pathsOrSheet.get_type() == sol::type::lua_nil) {
            return false;
        }

        const float duration = frameDuration.value_or(0.1f);
        const bool looping = loop.value_or(true);
        const std::string clipName = "Idle";
        Animation animation;

        if (pathsOrSheet.is<std::string>()) {
            const std::string sheet = pathsOrSheet.as<std::string>();
            const float width = frameWidth.value_or(0.0f);
            const float height = frameHeight.value_or(0.0f);
            if (sheet.empty() || width <= 0.0f || height <= 0.0f) {
                return false;
            }
            const std::size_t total =
                totalFrames.value_or(0) > 0 ? static_cast<std::size_t>(*totalFrames) : 0;
            animation = Animation(sheet, width, height, total, duration, looping, 0.0f, 0.0f, 1.0f,
                                  1.0f, false, false, clipName);
        } else {
            std::vector<std::string> paths;
            if (!ReadPaths(pathsOrSheet, paths)) {
                return false;
            }
            animation = Animation(std::move(paths), duration, looping, 0.0f, 0.0f, 1.0f, 1.0f,
                                  false, false, clipName);
        }

        Internal::ensureTransform(registry, entity, false);
        if (registry.HasComponent<Animation>(entity)) {
            registry.RequireComponent<Animation>(entity) = std::move(animation);
        } else {
            registry.AddComponent<Animation>(entity, std::move(animation));
        }
        return true;
    };

    component["playAnimation"] = [&registry](Entity entity, const std::string& clipName,
                                             sol::optional<bool> restartIfSame) -> bool {
        Animation* animation = FindAnimation(registry, entity);
        return animation != nullptr && animation->Play(clipName, restartIfSame.value_or(false));
    };

    component["getAnimation"] = [luaState = lua.lua_state(),
                                 &registry](Entity entity) mutable -> sol::object {
        sol::state_view view(luaState);
        Animation* animation = FindAnimation(registry, entity);
        if (animation == nullptr || animation->currentAnimation.empty()) {
            return sol::make_object(view, sol::lua_nil);
        }
        return sol::make_object(view, animation->currentAnimation);
    };

    component["getAnimationClips"] = [luaState = lua.lua_state(),
                                      &registry](Entity entity) mutable -> sol::object {
        sol::state_view view(luaState);
        Animation* animation = FindAnimation(registry, entity);
        if (animation == nullptr) {
            return sol::make_object(view, sol::lua_nil);
        }
        sol::table clips = view.create_table();
        std::size_t index = 0;
        for (const auto& [name, clip] : animation->clips) {
            (void)clip;
            clips[++index] = name;
        }
        return sol::make_object(view, clips);
    };

    component["setAnimationPlaying"] = [&registry](Entity entity, bool playing) -> bool {
        Animation* animation = FindAnimation(registry, entity);
        if (animation == nullptr) {
            return false;
        }
        animation->isPlaying = playing;
        return true;
    };

    component["isAnimationPlaying"] = [&registry](Entity entity) -> bool {
        Animation* animation = FindAnimation(registry, entity);
        return animation != nullptr && animation->isPlaying;
    };

    component["getAnimationFrame"] = [luaState = lua.lua_state(),
                                      &registry](Entity entity) mutable -> sol::object {
        sol::state_view view(luaState);
        Animation* animation = FindAnimation(registry, entity);
        if (animation == nullptr) {
            return sol::make_object(view, sol::lua_nil);
        }
        return sol::make_object(view, static_cast<double>(animation->currentFrame));
    };

    component["setAnimationFlip"] = [&registry](Entity entity, bool flipX,
                                                sol::optional<bool> flipY) -> bool {
        Animation* animation = FindAnimation(registry, entity);
        if (animation == nullptr) {
            return false;
        }
        animation->flipX = flipX;
        if (flipY.has_value()) {
            animation->flipY = flipY.value();
        }
        return true;
    };

    component["getAnimationFlip"] = [luaState = lua.lua_state(),
                                     &registry](Entity entity) mutable -> sol::object {
        sol::state_view view(luaState);
        Animation* animation = FindAnimation(registry, entity);
        if (animation == nullptr) {
            return sol::make_object(view, sol::lua_nil);
        }
        sol::table flip = view.create_table();
        flip["flipX"] = animation->flipX;
        flip["flipY"] = animation->flipY;
        return sol::make_object(view, flip);
    };
}

} // namespace Engine::LuaAPI
