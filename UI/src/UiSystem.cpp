#include "UI/UiSystem.hpp"
#include "Assets/ProjectPaths.hpp"
#include "Components/Position.hpp"
#include "Core/EditorMode.hpp"
#include "Core/QuitRequest.hpp"
#include "Core/VirtualResolution.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include "Resources/SceneManager.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Scripting/ScriptSystem.hpp"
#include "Scripting/SignalDispatch.hpp"
#include "Systems/CameraSystem.hpp"
#include "UI/UiCatalog.hpp"
#include "UI/UiComponent.hpp"
#include "UI/UiNavigation.hpp"
#include "UI/UiRuntime.hpp"
#include <array>
#include <cmath>
#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h>
#include <memory>
#include <optional>
#include <typeindex>
#include <vector>

namespace Engine {

std::unordered_set<std::string> UiSystem::s_WarnedMissing;

namespace {

constexpr ImGuiWindowFlags kWidgetWindowFlags =
    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

// Above this, a double no longer holds every integer exactly.
constexpr double kLargestWholeNumber = 9007199254740992.0;

// GetSystem throws when the script plugin was not loaded.
std::shared_ptr<ScriptSystem> FindScriptSystem(Registry& registry) {
    const auto& systems = registry.GetSystems();
    const auto it = systems.find(typeid(ScriptSystem));
    if (it == systems.end()) {
        return nullptr;
    }
    return std::static_pointer_cast<ScriptSystem>(it->second);
}

UiNavigation& Navigation(Registry& registry) {
    if (!registry.HasResource<UiNavigation>()) {
        registry.AddResource<UiNavigation>();
    }
    return registry.GetResource<UiNavigation>();
}

// Picks the start page on the first draw that finds one, so screens spawned
// after the first frame still open.
std::string CurrentPage(Registry& registry) {
    UiNavigation& navigation = Navigation(registry);

    if (!navigation.started) {
        navigation.current = StartUiPage(registry);
        navigation.started = !navigation.current.empty();
    }
    return navigation.current;
}

bool HasUi(Registry& registry, Entity entity) {
    return registry.IsComponentRegistered<UiComponent>() && registry.IsEntityAlive(entity) &&
           registry.HasComponent<UiComponent>(entity);
}

std::string PropString(const std::vector<UiProp>& props, const std::string& key) {
    std::optional<std::size_t> index = FindUiPropIndex(props, key);

    if (!index.has_value()) {
        return "";
    }
    return props[index.value()].stringValue;
}

void CallMethod(Registry& registry, Entity entity, const std::string& method) {
    sol::optional<sol::function> hook;
    sol::protected_function_result result;

    if (method.empty() || registry.HasResource<EditorMode>()) {
        return;
    }
    if (!registry.IsComponentRegistered<ScriptComponent>() ||
        !registry.HasComponent<ScriptComponent>(entity)) {
        return;
    }
    auto& script = registry.RequireComponent<ScriptComponent>(entity);
    if (!script.loaded || !script.scriptTable.valid()) {
        return;
    }
    hook = script.scriptTable[method];
    if (!hook || !hook->valid()) {
        return;
    }
    result = hook->call(script.scriptTable, entity);
    if (!result.valid()) {
        sol::error err = result;
        log_user_error(std::string("[UI] ") + method + " error (entity " + std::to_string(entity) +
                       "): " + err.what());
    }
}

void RunAction(Registry& registry, Entity entity, const std::vector<UiProp>& props,
               const std::string& key, const std::string& signal) {
    CallMethod(registry, entity, PropString(props, key));
    Signals::EmitSignal(registry, entity, signal, entity);
}

void RunUiAction(Registry& registry, Entity entity, const UiAction& action) {
    if (registry.HasResource<EditorMode>()) {
        return;
    }
    UiNavigation& navigation = Navigation(registry);
    switch (action.type) {
        case UiActionType::GoToPage:
            if (!action.target.empty()) {
                navigation.history.push_back(navigation.current);
                navigation.current = action.target;
                navigation.started = true;
            }
            break;
        case UiActionType::Back:
            if (!navigation.history.empty()) {
                navigation.current = navigation.history.back();
                navigation.history.pop_back();
            }
            break;
        case UiActionType::CallMethod:
            CallMethod(registry, entity, action.target);
            break;
        case UiActionType::LoadScene:
            if (action.target.empty()) {
                log_user_error("[UI] Load scene: no scene set (entity " + std::to_string(entity) +
                               ")");
            } else if (registry.HasResource<SceneManager>()) {
                registry.GetResource<SceneManager>().Load(action.target);
            } else {
                log_user_error("[UI] Load scene " + action.target +
                               ": no SceneManager in this game");
            }
            break;
        case UiActionType::Quit:
            if (!registry.HasResource<QuitRequest>()) {
                registry.AddResource<QuitRequest>();
            }
            registry.GetResource<QuitRequest>().requested = true;
            break;
        case UiActionType::None:
            break;
    }
    Signals::EmitSignal(registry, entity, "onClick", entity);
}

// The click reads the action when it happens, so an edit made this frame counts.
void RunClick(Registry& registry, Entity entity) {
    UiAction action;

    if (!HasUi(registry, entity)) {
        return;
    }
    action = registry.RequireComponent<UiComponent>(entity).onClick;
    RunUiAction(registry, entity, action);
}

bool IsWholeNumber(double value) {
    return std::isfinite(value) && std::floor(value) == value &&
           std::fabs(value) <= kLargestWholeNumber;
}

sol::table BuildProps(sol::state_view lua, const std::vector<UiProp>& props) {
    sol::table table = lua.create_table();
    std::size_t index = 0;
    std::size_t itemIndex = 0;

    for (index = 0; index < props.size(); ++index) {
        const UiProp& prop = props[index];
        if (prop.kind == UiValueKind::Bool) {
            table[prop.key] = prop.boolValue;
        } else if (prop.kind == UiValueKind::Number) {
            // ImGui int parameters reject 1.0, so whole numbers reach Lua as integers.
            if (IsWholeNumber(prop.numberValue)) {
                table[prop.key] = static_cast<long long>(prop.numberValue);
            } else {
                table[prop.key] = prop.numberValue;
            }
        } else if (prop.kind == UiValueKind::StringList) {
            sol::table list = lua.create_table();
            for (itemIndex = 0; itemIndex < prop.stringList.size(); ++itemIndex) {
                list[itemIndex + 1] = prop.stringList[itemIndex];
            }
            table[prop.key] = list;
        } else if (prop.kind == UiValueKind::Color) {
            if (!prop.colorSet) {
                continue;
            }
            sol::table color = lua.create_table();
            color[1] = prop.colorValue[0];
            color[2] = prop.colorValue[1];
            color[3] = prop.colorValue[2];
            color[4] = prop.colorValue[3];
            table[prop.key] = color;
        } else {
            table[prop.key] = prop.stringValue;
        }
    }
    return table;
}

// Children drawn by a screen. Custom scripts under a screen still draw on their own.
std::vector<Entity> DrawnChildren(Registry& registry, Entity screen) {
    std::vector<Entity> children;

    for (const Entity child : FindUiScreenChildren(registry, screen)) {
        if (!registry.RequireComponent<UiComponent>(child).templateName.empty()) {
            children.push_back(child);
        }
    }
    return children;
}

// props.children: { template, props, onClick }. A button's onClick is also in its props.
sol::table BuildChildren(sol::state_view lua, Registry& registry,
                         const std::vector<Entity>& children) {
    sol::table list = lua.create_table();
    std::size_t index = 0;

    for (index = 0; index < children.size(); ++index) {
        const Entity child = children[index];
        const UiComponent& ui = registry.RequireComponent<UiComponent>(child);
        sol::table entry = lua.create_table();
        sol::table props = BuildProps(lua, ui.props);
        entry["template"] = ui.templateName;
        if (ui.templateName == "button") {
            entry.set_function("onClick", [&registry, child]() { RunClick(registry, child); });
            props["onClick"] = entry.get<sol::object>("onClick");
        }
        entry["props"] = props;
        list[index + 1] = entry;
    }
    return list;
}

void WriteReturned(std::vector<UiProp>& props, const sol::object& returned) {
    std::optional<std::size_t> index = FindUiPropIndex(props, "value");

    if (!index.has_value() || returned.get_type() == sol::type::lua_nil) {
        return;
    }
    if (returned.get_type() == sol::type::boolean &&
        props[index.value()].kind == UiValueKind::Bool) {
        props[index.value()].boolValue = returned.as<bool>();
        return;
    }
    if (returned.get_type() == sol::type::number &&
        props[index.value()].kind == UiValueKind::Number) {
        props[index.value()].numberValue = returned.as<double>();
        return;
    }
    if (returned.get_type() == sol::type::string &&
        props[index.value()].kind == UiValueKind::String) {
        props[index.value()].stringValue = returned.as<std::string>();
    }
}

bool ValueChanged(const std::vector<UiProp>& props, double previousNumber, bool previousBool,
                  const std::string& previousString, bool hadValue) {
    std::optional<std::size_t> index = FindUiPropIndex(props, "value");

    if (!hadValue || !index.has_value()) {
        return false;
    }
    if (props[index.value()].kind == UiValueKind::Bool) {
        return props[index.value()].boolValue != previousBool;
    }
    if (props[index.value()].kind == UiValueKind::Number) {
        return props[index.value()].numberValue != previousNumber;
    }
    if (props[index.value()].kind == UiValueKind::String) {
        return props[index.value()].stringValue != previousString;
    }
    return false;
}

void CaptureValue(const std::vector<UiProp>& props, double& previousNumber, bool& previousBool,
                  std::string& previousString, bool& hadValue) {
    std::optional<std::size_t> index = FindUiPropIndex(props, "value");

    hadValue = index.has_value();
    if (!hadValue) {
        return;
    }
    previousBool = props[index.value()].boolValue;
    previousNumber = props[index.value()].numberValue;
    previousString = props[index.value()].stringValue;
}

// Writes one returned value into its prop and runs onChange when it changed.
void WriteValue(Registry& registry, Entity entity, const sol::object& returned) {
    double previousNumber = 0.0;
    bool previousBool = false;
    std::string previousString;
    bool hadValue = false;

    auto& ui = registry.RequireComponent<UiComponent>(entity);
    CaptureValue(ui.props, previousNumber, previousBool, previousString, hadValue);
    WriteReturned(ui.props, returned);
    if (ValueChanged(ui.props, previousNumber, previousBool, previousString, hadValue)) {
        RunAction(registry, entity, ui.props, "onChange", "onChange");
    }
}

// A screen returns its children's values keyed by child index (1-based).
void WriteChildren(Registry& registry, const std::vector<Entity>& children,
                   const sol::table& values) {
    std::size_t index = 0;
    sol::object returned;

    for (index = 0; index < children.size(); ++index) {
        if (!HasUi(registry, children[index]) ||
            registry.RequireComponent<UiComponent>(children[index]).templateName == "button") {
            continue;
        }
        returned = values[index + 1];
        if (returned.get_type() == sol::type::lua_nil) {
            continue;
        }
        WriteValue(registry, children[index], returned);
    }
}

// Pops what a failed draw left open, in our style stack and in ImGui.
void RecoverDraw(Registry& registry, std::size_t depth, const ImGuiErrorRecoveryState& state) {
    ImGuiIO& io = ImGui::GetIO();
    const bool assertEnabled = io.ConfigErrorRecoveryEnableAssert;
    const bool tooltipEnabled = io.ConfigErrorRecoveryEnableTooltip;

    // Our frames go first, so ImGui finds its stacks already at the stored size
    // for them and pops nothing twice.
    io.ConfigErrorRecoveryEnableAssert = false;
    io.ConfigErrorRecoveryEnableTooltip = false;
    if (registry.HasResource<UiRuntime>()) {
        registry.GetResource<UiRuntime>().RestoreStyleDepth(depth);
    }
    ImGui::ErrorRecoveryTryToRecoverState(&state);
    io.ConfigErrorRecoveryEnableAssert = assertEnabled;
    io.ConfigErrorRecoveryEnableTooltip = tooltipEnabled;
}

// Every Lua draw goes through here. A Lua error is logged and the ImGui frame
// is put back as it was before the call, instead of aborting on a missing Pop.
template <typename Call>
sol::protected_function_result GuardedDraw(Registry& registry, const std::string& what, Call call) {
    ImGuiErrorRecoveryState state;
    std::size_t depth = 0;
    const bool hasContext = ImGui::GetCurrentContext() != nullptr;
    sol::protected_function_result result;

    if (registry.HasResource<UiRuntime>()) {
        depth = registry.GetResource<UiRuntime>().StyleDepth();
    }
    if (hasContext) {
        ImGui::ErrorRecoveryStoreState(&state);
    }
    result = call();
    if (result.valid()) {
        return result;
    }
    sol::error err = result;
    log_user_error("[UI] draw error (" + what + "): " + err.what());
    if (hasContext) {
        RecoverDraw(registry, depth, state);
    }
    return result;
}

void DrawTemplate(Registry& registry, Entity entity, UiComponent& ui) {
    std::shared_ptr<ScriptSystem> scripts = FindScriptSystem(registry);
    const UiTemplateInfo* info = FindUiTemplate(ui.templateName);
    const std::string templateName = ui.templateName;
    std::vector<Entity> children;
    sol::table props;
    sol::table templ;
    sol::optional<sol::protected_function> draw;
    sol::protected_function_result result;
    sol::object returned;
    UiAction action;

    if (!scripts || !registry.HasResource<UiRuntime>()) {
        return;
    }
    sol::state_view lua = scripts->GetLua();
    templ = registry.GetResource<UiRuntime>().UseTemplate(lua, registry, templateName);
    draw = templ["draw"];
    if (!draw || !draw->valid()) {
        return;
    }
    props = BuildProps(lua, ui.props);
    if (info != nullptr && IsUiScreen(*info)) {
        children = DrawnChildren(registry, entity);
        props["children"] = BuildChildren(lua, registry, children);
    }
    result = GuardedDraw(registry, templateName, [&draw, &props]() { return draw->call(props); });
    if (!result.valid() || result.return_count() < 1) {
        return;
    }
    returned = result.get<sol::object>();
    // A callback run during draw (a method, a click) may have removed the component.
    if (!HasUi(registry, entity)) {
        return;
    }
    if (info != nullptr && IsUiScreen(*info) && returned.get_type() == sol::type::table) {
        WriteChildren(registry, children, returned.as<sol::table>());
        return;
    }
    if (templateName == "button") {
        if (returned.get_type() == sol::type::boolean && returned.as<bool>()) {
            action = registry.RequireComponent<UiComponent>(entity).onClick;
            RunUiAction(registry, entity, action);
        }
        return;
    }
    if (templateName == "confirm") {
        if (returned.get_type() == sol::type::string && returned.as<std::string>() == "confirm") {
            action = registry.RequireComponent<UiComponent>(entity).onClick;
            RunUiAction(registry, entity, action);
        } else if (returned.get_type() == sol::type::string &&
                   returned.as<std::string>() == "cancel") {
            action = registry.RequireComponent<UiComponent>(entity).onCancel;
            RunUiAction(registry, entity, action);
        }
        return;
    }
    WriteValue(registry, entity, returned);
}

// Standalone widget: borderless window at its Position, mapped like the canvas.
void DrawWidgetWindow(Registry& registry, Entity entity) {
    const std::string id = "##ui-widget-" + std::to_string(entity);
    Vector2 world{0.0f, 0.0f};
    Vector2 canvas{0.0f, 0.0f};
    ImVec2 window{0.0f, 0.0f};
    Rectangle dest{};
    float scale = 1.0f;

    if (registry.HasComponent<Position>(entity)) {
        world.x = registry.RequireComponent<Position>(entity).x;
        world.y = registry.RequireComponent<Position>(entity).y;
    }
    canvas = CameraSystem::WorldToScreen(registry, world);
    window = ImVec2(canvas.x, canvas.y);
    if (registry.HasResource<VirtualResolution>()) {
        dest = registry.GetResource<VirtualResolution>().GetDestRect();
        scale = registry.GetResource<VirtualResolution>().GetScale();
        window = ImVec2(dest.x + canvas.x * scale, dest.y + canvas.y * scale);
    }
    ImGui::SetNextWindowPos(window);
    // No padding, so the widget's corner sits on its Position.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin(id.c_str(), nullptr, kWidgetWindowFlags);
    ImGui::PopStyleVar();
    DrawTemplate(registry, entity, registry.RequireComponent<UiComponent>(entity));
    ImGui::End();
}

// Editor draws run in preview mode, then the previous mode is put back.
void DrawPreview(Registry& registry, Entity entity, const std::array<float, 4>& rect) {
    UiRuntime& runtime = registry.GetResource<UiRuntime>();
    const bool wasPreview = runtime.IsPreview();
    const std::array<float, 4> previousRect = runtime.GetScreenRect();

    runtime.SetPreview(true, rect[0], rect[1], rect[2], rect[3]);
    DrawTemplate(registry, entity, registry.RequireComponent<UiComponent>(entity));
    runtime.SetPreview(wasPreview, previousRect[0], previousRect[1], previousRect[2],
                       previousRect[3]);
}

void CallHook(sol::table table, const std::string& name, Entity entity) {
    sol::optional<sol::function> hook = table[name];
    sol::protected_function_result result;

    if (!hook || !hook->valid()) {
        return;
    }
    result = hook->call(table, entity);
    if (!result.valid()) {
        sol::error err = result;
        log_user_error(std::string("[UI] ") + name + " error (entity " + std::to_string(entity) +
                       "): " + err.what());
    }
}

} // namespace

void UiSystem::Draw(Registry& registry) {
    std::shared_ptr<ScriptSystem> scripts = FindScriptSystem(registry);
    const UiTemplateInfo* info = nullptr;
    std::vector<Entity> widgets;
    std::string current;

    if (!scripts || !scripts->IsApiReady()) {
        return;
    }
    if (!registry.IsComponentRegistered<UiComponent>()) {
        return;
    }
    current = CurrentPage(registry);

    // Reset per-frame hover state so backgrounds can fall back to stage 1.
    if (scripts && scripts->IsApiReady()) {
        scripts->GetLua()["hoveredThisFrame"] = false;
    }

    // Screens and custom scripts first, so standalone widgets sit on top.
    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Entity entity{i};
        sol::optional<sol::protected_function> draw;

        if (!registry.IsEntityAlive(entity) || !registry.HasComponent<UiComponent>(entity)) {
            continue;
        }
        auto& ui = registry.RequireComponent<UiComponent>(entity);
        if (!ui.templateName.empty()) {
            info = FindUiTemplate(ui.templateName);
            if (info != nullptr && IsUiScreen(*info)) {
                if (ui.page == current) {
                    DrawTemplate(registry, entity, ui);
                }
            } else if (!FindUiScreenParent(registry, entity).has_value() &&
                       (ui.showOn.empty() || ui.showOn == current)) {
                widgets.push_back(entity);
            }
            continue;
        }
        if (!ui.loaded && !ui.scriptPath.empty()) {
            LoadScript(registry, entity);
        }
        if (!ui.loaded || !ui.scriptTable.valid()) {
            continue;
        }
        draw = ui.scriptTable["draw"];
        if (!draw || !draw->valid()) {
            continue;
        }
        // One bad script must not skip the other entities this frame.
        GuardedDraw(registry, "entity " + std::to_string(entity),
                    [&draw, &ui, entity]() { return draw->call(ui.scriptTable, entity); });
    }
    for (const Entity entity : widgets) {
        if (HasUi(registry, entity)) {
            DrawWidgetWindow(registry, entity);
        }
    }

    // If no level button was hovered this frame, restore the stage 1 background.
    if (scripts && scripts->IsApiReady()) {
        sol::state_view lua = scripts->GetLua();
        bool hovered = false;
        sol::object hoveredObj = lua["hoveredThisFrame"];
        if (hoveredObj.is<bool>()) {
            hovered = hoveredObj.as<bool>();
        }
        if (!hovered) {
            sol::optional<sol::function> fn = lua["setLevelBackground"];
            if (fn && fn->valid()) {
                (*fn)(1);
            }
        }
    }
}

void UiSystem::DrawInPlace(Registry& registry, Entity entity) {
    std::shared_ptr<ScriptSystem> scripts = FindScriptSystem(registry);
    const UiTemplateInfo* info = nullptr;

    if (!scripts || !scripts->IsApiReady() || !registry.HasResource<UiRuntime>()) {
        return;
    }
    if (!registry.IsComponentRegistered<UiComponent>() ||
        !registry.HasComponent<UiComponent>(entity)) {
        return;
    }
    auto& ui = registry.RequireComponent<UiComponent>(entity);
    if (ui.templateName.empty()) {
        return;
    }
    info = FindUiTemplate(ui.templateName);
    if (info == nullptr || !IsUiWidget(*info)) {
        return;
    }
    DrawPreview(registry, entity, registry.GetResource<UiRuntime>().GetScreenRect());
}

void UiSystem::DrawPagePreview(Registry& registry, const std::string& page, float x, float y,
                               float width, float height) {
    std::shared_ptr<ScriptSystem> scripts = FindScriptSystem(registry);
    std::optional<Entity> screen;

    if (!scripts || !scripts->IsApiReady() || !registry.HasResource<UiRuntime>()) {
        return;
    }
    screen = FindUiPageScreen(registry, page);
    if (!screen.has_value()) {
        return;
    }
    DrawPreview(registry, screen.value(), {x, y, width, height});
}

void UiSystem::LoadScript(Registry& registry, Entity entity) {
    auto& ui = registry.RequireComponent<UiComponent>(entity);
    std::shared_ptr<ScriptSystem> scripts = FindScriptSystem(registry);
    std::filesystem::path fullPath;
    sol::protected_function_result result;
    sol::object object;

    if (!scripts || !scripts->IsApiReady() || ui.scriptPath.empty()) {
        return;
    }
    fullPath = ResolveAssetPath(registry, ui.scriptPath);
    if (!std::filesystem::exists(fullPath)) {
        // The path is checked every frame. Warn only the first time.
        if (s_WarnedMissing.insert(ui.scriptPath).second) {
            log_user_error("[UI] Script not found: " + fullPath.string());
        }
        return;
    }

    result = scripts->GetLua().script_file(fullPath.string());
    if (!result.valid()) {
        sol::error err = result;
        log_user_error("[UI] Failed to load script " + fullPath.string() + ": " + err.what());
        return;
    }
    object = result;
    if (object.get_type() != sol::type::table) {
        log_user_error("[UI] Script must return a table: " + fullPath.string());
        return;
    }

    ui.scriptTable = object.as<sol::table>();
    ui.loaded = true;
    // onOpen runs once, after the table exists.
    CallHook(ui.scriptTable, "onOpen", entity);
    log_info("[UI] Loaded UI script for entity " + std::to_string(entity) + ": " + ui.scriptPath);
}

void UiSystem::UnloadScript(Registry& registry, Entity entity) {
    auto& ui = registry.RequireComponent<UiComponent>(entity);

    if (!ui.loaded || !ui.scriptTable.valid()) {
        ui.scriptTable = sol::lua_nil;
        ui.loaded = false;
        return;
    }
    CallHook(ui.scriptTable, "onClose", entity);
    ui.scriptTable = sol::lua_nil;
    ui.loaded = false;
}

void UiSystem::OnSceneUnloaded(Registry& registry) {
    std::vector<Entity> entities;
    std::shared_ptr<ComponentArray<UiComponent>> components;

    if (registry.HasResource<UiNavigation>()) {
        registry.GetResource<UiNavigation>() = UiNavigation{};
    }
    if (!registry.IsComponentRegistered<UiComponent>()) {
        return;
    }
    // onClose can remove the component, so the list is copied first.
    components = registry.GetComponentArray<UiComponent>();
    for (const auto& [entity, ui] : *components) {
        if (ui.loaded) {
            entities.push_back(entity);
        }
    }
    for (const Entity entity : entities) {
        if (registry.HasComponent<UiComponent>(entity)) {
            UnloadScript(registry, entity);
        }
    }
}

} // namespace Engine
