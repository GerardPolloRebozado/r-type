#include "Plugins/UiPluginEditor.hpp"
#include "Assets/ProjectPaths.hpp"
#include "Components/Name.hpp"
#include "EditorConfig.hpp"
#include "Panels/HierarchyPanel.hpp"
#include "Panels/StatePanel.hpp"
#include "UI/UiCatalog.hpp"
#include "UI/UiComponent.hpp"
#include "UI/UiPlugin.hpp"
#include <algorithm>
#include <extras/IconsFontAwesome6.h>
#include <filesystem>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <optional>
#include <string>
#include <vector>

namespace Editor {

namespace {

bool IsSkippedFolder(const std::filesystem::path& folder) {
    const std::string name = folder.filename().string();

    return name.empty() || name[0] == '.' || name == "build" || name.rfind("cmake-build", 0) == 0;
}

std::vector<std::string> ListProjectScenes(Engine::Registry& registry) {
    std::vector<std::string> scenes;
    std::string root;
    std::error_code error;

    if (registry.HasResource<EditorConfig>()) {
        root = registry.GetResource<EditorConfig>().m_ProjectFolder;
    } else if (registry.HasResource<Engine::ProjectContext>()) {
        root = registry.GetResource<Engine::ProjectContext>().projectFolder;
    }
    if (root.empty()) {
        return scenes;
    }
    std::filesystem::recursive_directory_iterator it(
        root, std::filesystem::directory_options::skip_permission_denied, error);
    for (const std::filesystem::recursive_directory_iterator end; !error && it != end;
         it.increment(error)) {
        if (it->is_directory(error)) {
            if (IsSkippedFolder(it->path())) {
                it.disable_recursion_pending();
            }
            continue;
        }
        if (it->path().extension() == ".scene") {
            scenes.push_back(std::filesystem::relative(it->path(), root, error).generic_string());
        }
    }
    std::sort(scenes.begin(), scenes.end());
    return scenes;
}

std::optional<std::string> AcceptBrowserPath() {
    const ImGuiPayload* dropped = nullptr;

    dropped = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM");
    if (dropped == nullptr || dropped->Data == nullptr) {
        return std::nullopt;
    }
    return std::string(static_cast<const char*>(dropped->Data));
}

void DrawStringList(const char* label, std::vector<std::string>& values) {
    int index = 0;
    int remove = -1;

    ImGui::TextUnformatted(label);
    for (index = 0; index < static_cast<int>(values.size()); ++index) {
        ImGui::PushID(index);
        ImGui::SetNextItemWidth(180.0f);
        ImGui::InputText("##item", &values[static_cast<std::size_t>(index)]);
        ImGui::SameLine();
        if (ImGui::SmallButton("-")) {
            remove = index;
        }
        ImGui::PopID();
    }
    if (remove >= 0) {
        values.erase(values.begin() + remove);
    }
    ImGui::PushID(label);
    if (ImGui::SmallButton("+")) {
        values.emplace_back();
    }
    ImGui::PopID();
}

void FieldLabel(const char* label) {
    ImGui::TextUnformatted(label);
    ImGui::SetNextItemWidth(-FLT_MIN);
}

void DrawField(Engine::Registry& registry, std::vector<Engine::UiProp>& props,
               const Engine::UiField& field) {
    Engine::UiProp& prop = Engine::EnsureUiProp(props, field);
    int whole = 0;
    float number = 0.0f;
    int option = 0;
    int selected = 0;
    std::optional<std::string> dropped;
    std::string key = field.key;

    if (field.type == Engine::UiFieldType::Bool) {
        ImGui::Checkbox(field.label, &prop.boolValue);
        return;
    }
    if (field.type == Engine::UiFieldType::Number && field.integer) {
        FieldLabel(field.label);
        whole = static_cast<int>(prop.numberValue);
        if (ImGui::InputInt("##number", &whole)) {
            prop.numberValue = whole;
        }
        return;
    }
    if (field.type == Engine::UiFieldType::Number) {
        FieldLabel(field.label);
        number = static_cast<float>(prop.numberValue);
        if (ImGui::InputFloat("##number", &number)) {
            prop.numberValue = number;
        }
        return;
    }
    if (field.type == Engine::UiFieldType::Color) {
        ImGui::Checkbox(field.label, &prop.colorSet);
        if (prop.colorSet) {
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::ColorEdit4("##color", prop.colorValue.data(), ImGuiColorEditFlags_AlphaBar);
        }
        return;
    }
    if (field.type == Engine::UiFieldType::StringList) {
        DrawStringList(field.label, prop.stringList);
        return;
    }
    if (field.type == Engine::UiFieldType::Enum) {
        FieldLabel(field.label);
        for (option = 0; option < field.optionCount; ++option) {
            if (prop.stringValue == field.options[option]) {
                selected = option;
            }
        }
        if (ImGui::BeginCombo("##enum", prop.stringValue.c_str())) {
            for (option = 0; option < field.optionCount; ++option) {
                if (ImGui::Selectable(field.options[option], option == selected)) {
                    prop.stringValue = field.options[option];
                }
            }
            ImGui::EndCombo();
        }
        return;
    }

    FieldLabel(field.label);
    ImGui::InputText("##text", &prop.stringValue);
    if (key == "onChange") {
        ImGui::TextWrapped("Method on this entity's script. Empty does nothing.");
    }
    if (key == "path" && ImGui::BeginDragDropTarget()) {
        dropped = AcceptBrowserPath();
        if (dropped.has_value()) {
            prop.stringValue = ToProjectRelativePath(registry, *dropped);
        }
        ImGui::EndDragDropTarget();
    }
}

void DrawFields(Engine::Registry& registry, std::vector<Engine::UiProp>& props,
                const Engine::UiTemplateInfo& info) {
    int index = 0;

    for (index = 0; index < info.fieldCount; ++index) {
        ImGui::PushID(info.fields[index].key);
        DrawField(registry, props, info.fields[index]);
        ImGui::PopID();
    }
}

void UseCustomScript(Engine::UiComponent& ui) {
    ui.templateName.clear();
    ui.props.clear();
    ui.loaded = false;
    ui.scriptTable = sol::lua_nil;
}

void SelectEntity(Engine::Registry& registry, Engine::Entity entity) {
    if (!registry.HasResource<HierarchyPanelSelectedEntity>()) {
        return;
    }
    auto& selected = registry.GetResource<HierarchyPanelSelectedEntity>();
    selected.entity = entity;
    selected.index = static_cast<int>(entity);
    if (registry.HasResource<StatePanelSelectedState>()) {
        auto& stateSel = registry.GetResource<StatePanelSelectedState>();
        stateSel.groupIndex = -1;
        stateSel.stateIndex = -1;
    }
}

// Action type, then the one control that type needs (page, method, or scene).
void DrawAction(Engine::Registry& registry, const char* label, Engine::UiAction& action) {
    std::vector<std::string> pages;
    std::vector<std::string> scenes;
    std::optional<std::string> dropped;
    Engine::UiActionType type = Engine::UiActionType::None;
    int index = 0;

    ImGui::PushID(label);
    ImGui::SeparatorText(label);
    FieldLabel("Action");
    if (ImGui::BeginCombo("##type", Engine::UiActionLabel(action.type))) {
        for (index = 0; index < Engine::kUiActionTypeCount; ++index) {
            type = static_cast<Engine::UiActionType>(index);
            if (ImGui::Selectable(Engine::UiActionLabel(type), action.type == type) &&
                action.type != type) {
                // A page name means nothing as a method or a scene, so the target restarts empty.
                action.type = type;
                action.target.clear();
            }
        }
        ImGui::EndCombo();
    }
    if (action.type == Engine::UiActionType::GoToPage) {
        pages = Engine::ListUiPages(registry);
        if (pages.empty()) {
            ImGui::TextDisabled("(no pages yet)");
        } else {
            FieldLabel("Page");
            if (ImGui::BeginCombo("##page", action.target.empty() ? "Choose a page"
                                                                  : action.target.c_str())) {
                for (const std::string& page : pages) {
                    if (ImGui::Selectable(page.c_str(), action.target == page)) {
                        action.target = page;
                    }
                }
                ImGui::EndCombo();
            }
        }
    } else if (action.type == Engine::UiActionType::CallMethod) {
        FieldLabel("Method");
        ImGui::InputText("##method", &action.target);
        ImGui::TextWrapped("Method on this entity's script");
    } else if (action.type == Engine::UiActionType::LoadScene) {
        scenes = ListProjectScenes(registry);
        FieldLabel("Scene");
        if (scenes.empty()) {
            ImGui::InputText("##scene", &action.target);
            ImGui::TextWrapped("No .scene files in this project yet.");
        } else if (ImGui::BeginCombo("##scene", action.target.empty() ? "Choose a scene"
                                                                      : action.target.c_str())) {
            for (const std::string& scene : scenes) {
                if (ImGui::Selectable(scene.c_str(), action.target == scene)) {
                    action.target = scene;
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::BeginDragDropTarget()) {
            dropped = AcceptBrowserPath();
            if (dropped.has_value()) {
                action.target = ToProjectRelativePath(registry, *dropped);
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::TextWrapped("Opens that scene when the game is running.");
    }
    ImGui::PopID();
}

// A widget under a screen belongs to that page. Only standalone widgets pick one.
void DrawShowOn(Engine::Registry& registry, Engine::Entity entity, Engine::UiComponent& ui) {
    std::optional<Engine::Entity> screen = Engine::FindUiScreenParent(registry, entity);
    std::vector<std::string> pages;

    if (screen.has_value()) {
        ImGui::TextWrapped(
            "Part of page: %s",
            registry.RequireComponent<Engine::UiComponent>(screen.value()).page.c_str());
        return;
    }
    pages = Engine::ListUiPages(registry);
    FieldLabel("Show on");
    if (ImGui::BeginCombo("##show-on", ui.showOn.empty() ? "Always" : ui.showOn.c_str())) {
        if (ImGui::Selectable("Always", ui.showOn.empty())) {
            ui.showOn.clear();
        }
        for (const std::string& page : pages) {
            if (ImGui::Selectable(page.c_str(), ui.showOn == page)) {
                ui.showOn = page;
            }
        }
        ImGui::EndCombo();
    }
}

void DrawContents(Engine::Registry& registry, Engine::Entity screen) {
    std::vector<Engine::Entity> children = Engine::FindUiScreenChildren(registry, screen);
    const Engine::UiTemplateInfo* info = nullptr;
    std::optional<Engine::Entity> moved;
    std::optional<Engine::Entity> removed;
    std::string row;
    std::size_t index = 0;
    int option = 0;
    int direction = 0;

    ImGui::SeparatorText("Contents");
    if (children.empty()) {
        ImGui::TextDisabled("No widgets on this page yet.");
    }
    for (index = 0; index < children.size(); ++index) {
        const Engine::Entity child = children[index];
        info = Engine::FindUiTemplate(
            registry.RequireComponent<Engine::UiComponent>(child).templateName);
        row = Engine::GetEntityName(registry, child) + " (" +
              (info != nullptr ? info->label : "Custom script") + ")";
        ImGui::PushID(static_cast<int>(child));
        ImGui::BeginDisabled(index == 0);
        if (ImGui::ArrowButton("##up", ImGuiDir_Up)) {
            moved = child;
            direction = -1;
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(index + 1 == children.size());
        if (ImGui::ArrowButton("##down", ImGuiDir_Down)) {
            moved = child;
            direction = 1;
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Select")) {
            SelectEntity(registry, child);
        }
        ImGui::SameLine();
        if (ImGui::Button("x")) {
            removed = child;
        }
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(row.c_str());
        ImGui::PopID();
    }
    if (moved.has_value()) {
        Engine::MoveUiChild(registry, moved.value(), direction);
    }
    if (removed.has_value()) {
        registry.DestroyEntity(removed.value());
        if (registry.HasResource<HierarchyPanelSelectedEntity>()) {
            auto& selected = registry.GetResource<HierarchyPanelSelectedEntity>();
            if (selected.entity == removed.value() || !registry.IsEntityAlive(selected.entity)) {
                selected.entity = static_cast<Engine::Entity>(-1);
                selected.index = -1;
            }
        }
    }
    if (ImGui::Button("+ Add")) {
        ImGui::OpenPopup("add-ui-child");
    }
    if (ImGui::BeginPopup("add-ui-child")) {
        for (option = 0; option < Engine::UiTemplateCount(); ++option) {
            const Engine::UiTemplateInfo& widget = Engine::UiTemplates()[option];
            if (!Engine::IsUiWidget(widget)) {
                continue;
            }
            if (ImGui::MenuItem(widget.label)) {
                Engine::AddUiChild(registry, screen, widget.id);
            }
        }
        ImGui::EndPopup();
    }
}

} // namespace

const char* UiPluginEditor::GetName() const {
    return "UI";
}

const char* UiPluginEditor::GetIcon() const {
    return ICON_FA_DISPLAY;
}

void UiPluginEditor::DrawPageName(Engine::Registry& registry, Engine::Entity entity) {
    std::vector<std::string> pages;
    bool taken = false;
    bool committed = false;

    auto& ui = registry.RequireComponent<Engine::UiComponent>(entity);
    if (!m_PageEditing || m_PageEntity != entity) {
        m_PageBuffer = ui.page;
        m_PageEntity = entity;
    }
    FieldLabel("Page name");
    ImGui::InputText("##page-name", &m_PageBuffer);
    m_PageEditing = ImGui::IsItemActive();
    committed = ImGui::IsItemDeactivatedAfterEdit();
    if (m_PageBuffer != ui.page) {
        pages = Engine::ListUiPages(registry);
        // Two screens with one name would share a page.
        taken = std::find(pages.begin(), pages.end(), m_PageBuffer) != pages.end();
    }
    if (taken) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "Another page already has this name");
    }
    if (!committed || m_PageBuffer.empty() || taken || m_PageBuffer == ui.page) {
        return;
    }
    if (ui.page.empty()) {
        ui.page = m_PageBuffer;
        return;
    }
    Engine::RenameUiPage(registry, ui.page, m_PageBuffer);
}

bool UiPluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    std::string header;
    std::string pageBase;
    bool isOpen = false;
    bool startPage = false;
    int index = 0;
    const Engine::UiTemplateInfo* current = nullptr;
    std::optional<std::string> dropped;

    if (!registry.IsComponentRegistered<Engine::UiComponent>() ||
        !registry.HasComponent<Engine::UiComponent>(entity)) {
        return false;
    }

    auto& ui = registry.RequireComponent<Engine::UiComponent>(entity);
    header = std::string(GetIcon()) + " " + GetName();
    isOpen = ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Remove Component")) {
            RemoveComponent(entity, registry);
            ImGui::EndPopup();
            return false;
        }
        ImGui::EndPopup();
    }

    if (!isOpen) {
        return true;
    }

    current = Engine::FindUiTemplate(ui.templateName);
    ImGui::TextUnformatted("Template");
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##template", current != nullptr ? current->label : "Custom script")) {
        if (ImGui::Selectable("Custom script", ui.templateName.empty())) {
            if (!ui.templateName.empty()) {
                UseCustomScript(ui);
            }
            current = nullptr;
        }
        for (index = 0; index < Engine::UiTemplateCount(); ++index) {
            const Engine::UiTemplateInfo& info = Engine::UiTemplates()[index];
            if (ImGui::Selectable(info.label, ui.templateName == info.id)) {
                if (ui.templateName != info.id) {
                    Engine::ApplyUiTemplate(ui, info.id);
                    if (Engine::IsUiScreen(info) && ui.page.empty()) {
                        pageBase = info.label;
                        if (registry.IsComponentRegistered<Engine::Name>() &&
                            registry.HasComponent<Engine::Name>(entity) &&
                            !registry.RequireComponent<Engine::Name>(entity).value.empty()) {
                            pageBase = registry.RequireComponent<Engine::Name>(entity).value;
                        }
                        ui.page = Engine::UniqueUiPageName(registry, pageBase);
                    }
                }
                current = &info;
            }
        }
        ImGui::EndCombo();
    }

    if (ui.templateName.empty()) {
        ImGui::TextUnformatted("Script");
        ImGui::SetNextItemWidth(-FLT_MIN);
        // The loaded table belongs to the previous file. Drop it so the next frame reloads.
        if (ImGui::InputText("##script", &ui.scriptPath)) {
            ui.loaded = false;
            ui.scriptTable = sol::lua_nil;
        }
        if (ImGui::BeginDragDropTarget()) {
            dropped = AcceptBrowserPath();
            if (dropped.has_value()) {
                ui.scriptPath = ToProjectRelativePath(registry, *dropped);
                ui.loaded = false;
                ui.scriptTable = sol::lua_nil;
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::Checkbox("Auto Reload", &ui.autoReload);
        if (!ui.scriptPath.empty() &&
            !std::filesystem::exists(ResolveAssetPath(registry, ui.scriptPath))) {
            ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "File not found");
        }
        return true;
    }

    if (current == nullptr) {
        ImGui::TextDisabled("Unknown template");
        return true;
    }
    if (Engine::IsUiScreen(*current)) {
        ImGui::TextDisabled("This screen fills the game window.");
        DrawPageName(registry, entity);
        startPage = ui.startPage;
        if (ImGui::Checkbox("Start page", &startPage)) {
            if (startPage) {
                Engine::SetUiStartPage(registry, entity);
            } else {
                ui.startPage = false;
            }
        }
    } else {
        DrawShowOn(registry, entity, ui);
    }
    DrawFields(registry, ui.props, *current);
    if (ui.templateName == "button" || ui.templateName == "confirm") {
        DrawAction(registry, "On click", ui.onClick);
    }
    if (ui.templateName == "confirm") {
        DrawAction(registry, "On cancel", ui.onCancel);
    }
    if (Engine::IsUiScreen(*current)) {
        DrawContents(registry, entity);
    }
    return true;
}

bool UiPluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.IsComponentRegistered<Engine::UiComponent>()) {
        return false;
    }
    return registry.HasComponent<Engine::UiComponent>(entity);
}

void UiPluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    Engine::UiPlugin::AttachToEntity(registry, entity, Engine::UiComponent{});
}

void UiPluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.IsComponentRegistered<Engine::UiComponent>()) {
        return;
    }
    registry.RemoveComponent<Engine::UiComponent>(entity);
}

bool TryDrawUiAddMenu(IPluginEditor& editor, Engine::Entity entity, Engine::Registry& registry) {
    int index = 0;
    bool isScreen = false;
    bool hasUi = false;
    bool addsChild = false;
    std::string group;
    std::string menu;

    if (dynamic_cast<UiPluginEditor*>(&editor) == nullptr) {
        return false;
    }
    hasUi = editor.HasComponent(entity, registry);
    isScreen = Engine::IsUiScreenEntity(registry, entity);
    // Callers used to skip this once UI was present, so Add-to-page never ran.
    if (hasUi && !isScreen) {
        return false;
    }
    menu = std::string(editor.GetIcon()) + " Add " + editor.GetName();
    if (!ImGui::BeginMenu(menu.c_str())) {
        return true;
    }
    for (index = 0; index < Engine::UiTemplateCount(); ++index) {
        const Engine::UiTemplateInfo& info = Engine::UiTemplates()[index];
        // On a screen, widgets become children of its page instead of replacing it.
        addsChild = isScreen && Engine::IsUiWidget(info);
        if (hasUi && !addsChild) {
            continue;
        }
        if (group != info.group) {
            group = info.group;
            ImGui::SeparatorText(addsChild ? "Add to page" : group.c_str());
        }
        if (ImGui::MenuItem(info.label)) {
            if (addsChild) {
                Engine::AddUiChild(registry, entity, info.id);
            } else {
                Engine::AttachUiTemplate(registry, entity, info.id);
            }
        }
    }
    if (!hasUi) {
        ImGui::Separator();
        if (ImGui::MenuItem("Custom script")) {
            editor.AddComponent(entity, registry);
        }
    }
    ImGui::EndMenu();
    return true;
}

} // namespace Editor
