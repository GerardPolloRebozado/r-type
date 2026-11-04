#include "UI/UiPlugin.hpp"
#include "Core/UiPass.hpp"
#include "Scripting/LuaBinder.hpp"
#include "UI/LuaAPI/UiAPI.hpp"
#include "UI/UiRuntime.hpp"
#include "UI/UiSystem.hpp"
#include <sol/sol.hpp>

#ifndef UI_TEMPLATES_DIR
#define UI_TEMPLATES_DIR ""
#endif

namespace Engine {

void UiPlugin::Init(Registry& registry) {
    registry.RegisterComponent<UiComponent>();
    registry.RegisterSystem<UiSystem>();

    if (!registry.HasResource<UiRuntime>()) {
        registry.AddResource<UiRuntime>();
    }
    registry.GetResource<UiRuntime>().SetTemplateRoot(UI_TEMPLATES_DIR);

    if (!registry.HasResource<LuaBinder>()) {
        registry.AddResource<LuaBinder>();
    }
    // ScriptSystem calls this once, on its shared state, after the engine API.
    registry.GetResource<LuaBinder>().modules.push_back(
        [](sol::state_view state, Registry& bound) { LuaAPI::registerUI(state, bound); });

    if (!registry.HasResource<UiPass>()) {
        registry.AddResource<UiPass>();
    }
    // Window space, after the letterboxed canvas. The editor's render path never calls it.
    registry.GetResource<UiPass>().draw = [](Registry& bound) {
        if (!bound.HasResource<UiRuntime>()) {
            return;
        }
        bound.GetResource<UiRuntime>().Draw(bound);
    };
}

void UiPlugin::AttachToEntity(Registry& registry, Entity entity, const UiComponent& component) {
    registry.RegisterComponent<UiComponent>();
    registry.AddComponent(entity, component);
}

} // namespace Engine
