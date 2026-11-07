#include "UI/LuaAPI/UiAPI.hpp"
#include "UI/UiRuntime.hpp"
#include "sol_ImGui.h"
#include <array>
#include <tuple>

namespace Engine::LuaAPI {

void registerUI(sol::state_view lua, Registry& registry) {
    sol::table ui = lua.create_named_table("UI");

    // ImGui is the widget library. UI is the template layer on top of it.
    sol_ImGui::Init(lua);

    ui["use"] = [&registry, lua](const std::string& name) {
        return registry.GetResource<UiRuntime>().UseTemplate(lua, registry, name);
    };
    ui["register"] = [&registry](const std::string& name, sol::table table) {
        registry.GetResource<UiRuntime>().RegisterTemplate(name, std::move(table));
    };
    // create_table is non-const. A captured state view would otherwise be const.
    ui["merge"] = [lua](sol::table defaults, sol::optional<sol::table> props) mutable {
        sol::table merged = lua.create_table();
        defaults.for_each([&](sol::object key, sol::object value) { merged[key] = value; });
        if (props) {
            props->for_each([&](sol::object key, sol::object value) { merged[key] = value; });
        }
        return merged;
    };
    ui["setTheme"] = [&registry](sol::table props) {
        registry.GetResource<UiRuntime>().SetTheme(props);
    };
    ui["pushStyle"] = [&registry](sol::table props) {
        registry.GetResource<UiRuntime>().PushStyle(props);
    };
    ui["popStyle"] = [&registry]() { registry.GetResource<UiRuntime>().PopStyle(); };
    ui["blocksMouse"] = [&registry]() {
        if (!registry.HasResource<UiRuntime>()) {
            return false;
        }
        return registry.GetResource<UiRuntime>().BlocksMouse();
    };
    ui["blocksKeyboard"] = [&registry]() {
        if (!registry.HasResource<UiRuntime>()) {
            return false;
        }
        return registry.GetResource<UiRuntime>().BlocksKeyboard();
    };
    ui["image"] = [&registry](const std::string& path, float width, float height) {
        registry.GetResource<UiRuntime>().DrawImage(registry, path, width, height);
    };
    ui["beginScreen"] = [&registry](const std::string& id, sol::optional<int> flags) {
        if (!registry.HasResource<UiRuntime>()) {
            return;
        }
        registry.GetResource<UiRuntime>().BeginScreen(id, flags.value_or(0));
    };
    ui["endScreen"] = [&registry]() {
        if (!registry.HasResource<UiRuntime>()) {
            return;
        }
        registry.GetResource<UiRuntime>().EndScreen();
    };
    ui["beginPanel"] = [&registry](const std::string& id, float width, float height,
                                   sol::optional<int> flags) {
        if (!registry.HasResource<UiRuntime>()) {
            return;
        }
        registry.GetResource<UiRuntime>().BeginPanel(id, width, height, flags.value_or(0));
    };
    ui["endPanel"] = [&registry]() {
        if (!registry.HasResource<UiRuntime>()) {
            return;
        }
        registry.GetResource<UiRuntime>().EndPanel();
    };
    ui["screenSize"] = [&registry]() {
        std::array<float, 4> rect{0.0f, 0.0f, 0.0f, 0.0f};

        if (registry.HasResource<UiRuntime>()) {
            rect = registry.GetResource<UiRuntime>().GetScreenRect();
        }
        return std::make_tuple(rect[2], rect[3]);
    };
    ui["isPreview"] = [&registry]() {
        if (!registry.HasResource<UiRuntime>()) {
            return false;
        }
        return registry.GetResource<UiRuntime>().IsPreview();
    };
}

} // namespace Engine::LuaAPI
