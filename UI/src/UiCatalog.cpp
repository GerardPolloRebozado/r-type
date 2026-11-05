#include "UI/UiCatalog.hpp"
#include "Components/Name.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Systems/HierarchySystem.hpp"
#include "UI/UiNavigation.hpp"
#include "UI/UiPlugin.hpp"
#include <algorithm>
#include <cctype>
#include <memory>
#include <optional>
#include <string>

namespace Engine {
namespace {

constexpr const char* kStyles[] = {"primary", "secondary", "ghost", "danger"};
constexpr const char* kChoiceOptions[] = {"Easy", "Normal", "Hard"};
constexpr const char* kCameraOptions[] = {"Ship", "Wide"};
constexpr const char* kActionLabels[] = {"None",        "Go to page", "Back",
                                         "Call method", "Load scene", "Quit"};

constexpr UiField kButtonFields[] = {
    {.key = "label", .label = "Label", .type = UiFieldType::String, .stringDefault = "Button"},
    {.key = "style",
     .label = "Style",
     .type = UiFieldType::Enum,
     .stringDefault = "primary",
     .options = kStyles,
     .optionCount = 4},
    {.key = "width",
     .label = "Width",
     .type = UiFieldType::Number,
     .integer = true,
     .numberDefault = 280},
    {.key = "height",
     .label = "Height",
     .type = UiFieldType::Number,
     .integer = true,
     .numberDefault = 40},
    {.key = "color", .label = "Color", .type = UiFieldType::Color},
    {.key = "hover", .label = "Hover", .type = UiFieldType::Color},
    {.key = "active", .label = "Active", .type = UiFieldType::Color},
    {.key = "text", .label = "Text", .type = UiFieldType::Color},
};

constexpr UiField kCheckboxFields[] = {
    {.key = "label", .label = "Label", .type = UiFieldType::String, .stringDefault = "Option"},
    {.key = "value", .label = "Checked", .type = UiFieldType::Bool},
    {.key = "style",
     .label = "Style",
     .type = UiFieldType::Enum,
     .stringDefault = "primary",
     .options = kStyles,
     .optionCount = 4},
    {.key = "color", .label = "Color", .type = UiFieldType::Color},
    {.key = "hover", .label = "Hover", .type = UiFieldType::Color},
    {.key = "active", .label = "Active", .type = UiFieldType::Color},
    {.key = "text", .label = "Text", .type = UiFieldType::Color},
    {.key = "onChange", .label = "On change", .type = UiFieldType::String},
};

constexpr UiField kSliderFields[] = {
    {.key = "label", .label = "Label", .type = UiFieldType::String, .stringDefault = "Value"},
    {.key = "value", .label = "Value", .type = UiFieldType::Number, .numberDefault = 0.5},
    {.key = "min", .label = "Min", .type = UiFieldType::Number, .numberDefault = 0},
    {.key = "max", .label = "Max", .type = UiFieldType::Number, .numberDefault = 1},
    {.key = "style",
     .label = "Style",
     .type = UiFieldType::Enum,
     .stringDefault = "primary",
     .options = kStyles,
     .optionCount = 4},
    {.key = "color", .label = "Color", .type = UiFieldType::Color},
    {.key = "hover", .label = "Hover", .type = UiFieldType::Color},
    {.key = "active", .label = "Active", .type = UiFieldType::Color},
    {.key = "text", .label = "Text", .type = UiFieldType::Color},
    {.key = "onChange", .label = "On change", .type = UiFieldType::String},
};

constexpr UiField kSliderIntFields[] = {
    {.key = "label", .label = "Label", .type = UiFieldType::String, .stringDefault = "Count"},
    {.key = "value",
     .label = "Value",
     .type = UiFieldType::Number,
     .integer = true,
     .numberDefault = 1},
    {.key = "min", .label = "Min", .type = UiFieldType::Number, .integer = true, .numberDefault = 0},
    {.key = "max",
     .label = "Max",
     .type = UiFieldType::Number,
     .integer = true,
     .numberDefault = 10},
    {.key = "style",
     .label = "Style",
     .type = UiFieldType::Enum,
     .stringDefault = "primary",
     .options = kStyles,
     .optionCount = 4},
    {.key = "color", .label = "Color", .type = UiFieldType::Color},
    {.key = "hover", .label = "Hover", .type = UiFieldType::Color},
    {.key = "active", .label = "Active", .type = UiFieldType::Color},
    {.key = "text", .label = "Text", .type = UiFieldType::Color},
    {.key = "onChange", .label = "On change", .type = UiFieldType::String},
};

constexpr UiField kInputFields[] = {
    {.key = "label", .label = "Label", .type = UiFieldType::String, .stringDefault = "Text"},
    {.key = "value", .label = "Value", .type = UiFieldType::String},
    {.key = "width",
     .label = "Width",
     .type = UiFieldType::Number,
     .integer = true,
     .numberDefault = 220},
    {.key = "style",
     .label = "Style",
     .type = UiFieldType::Enum,
     .stringDefault = "primary",
     .options = kStyles,
     .optionCount = 4},
    {.key = "color", .label = "Color", .type = UiFieldType::Color},
    {.key = "hover", .label = "Hover", .type = UiFieldType::Color},
    {.key = "active", .label = "Active", .type = UiFieldType::Color},
    {.key = "text", .label = "Text", .type = UiFieldType::Color},
    {.key = "onChange", .label = "On change", .type = UiFieldType::String},
};

constexpr UiField kComboFields[] = {
    {.key = "label", .label = "Label", .type = UiFieldType::String, .stringDefault = "Choice"},
    {.key = "value",
     .label = "Value",
     .type = UiFieldType::Number,
     .integer = true,
     .numberDefault = 1},
    {.key = "options",
     .label = "Options",
     .type = UiFieldType::StringList,
     .listDefault = kChoiceOptions,
     .listCount = 3},
    {.key = "width",
     .label = "Width",
     .type = UiFieldType::Number,
     .integer = true,
     .numberDefault = 220},
    {.key = "style",
     .label = "Style",
     .type = UiFieldType::Enum,
     .stringDefault = "primary",
     .options = kStyles,
     .optionCount = 4},
    {.key = "color", .label = "Color", .type = UiFieldType::Color},
    {.key = "hover", .label = "Hover", .type = UiFieldType::Color},
    {.key = "active", .label = "Active", .type = UiFieldType::Color},
    {.key = "text", .label = "Text", .type = UiFieldType::Color},
    {.key = "onChange", .label = "On change", .type = UiFieldType::String},
};

constexpr UiField kRadioFields[] = {
    {.key = "label", .label = "Label", .type = UiFieldType::String, .stringDefault = "Choice"},
    {.key = "value",
     .label = "Value",
     .type = UiFieldType::Number,
     .integer = true,
     .numberDefault = 1},
    {.key = "options",
     .label = "Options",
     .type = UiFieldType::StringList,
     .listDefault = kCameraOptions,
     .listCount = 2},
    {.key = "style",
     .label = "Style",
     .type = UiFieldType::Enum,
     .stringDefault = "primary",
     .options = kStyles,
     .optionCount = 4},
    {.key = "color", .label = "Color", .type = UiFieldType::Color},
    {.key = "hover", .label = "Hover", .type = UiFieldType::Color},
    {.key = "active", .label = "Active", .type = UiFieldType::Color},
    {.key = "text", .label = "Text", .type = UiFieldType::Color},
    {.key = "onChange", .label = "On change", .type = UiFieldType::String},
};

constexpr UiField kProgressFields[] = {
    {.key = "label", .label = "Label", .type = UiFieldType::String},
    {.key = "value", .label = "Value", .type = UiFieldType::Number},
    {.key = "width",
     .label = "Width",
     .type = UiFieldType::Number,
     .integer = true,
     .numberDefault = 280},
    {.key = "height",
     .label = "Height",
     .type = UiFieldType::Number,
     .integer = true,
     .numberDefault = 22},
    {.key = "style",
     .label = "Style",
     .type = UiFieldType::Enum,
     .stringDefault = "primary",
     .options = kStyles,
     .optionCount = 4},
    {.key = "color", .label = "Color", .type = UiFieldType::Color},
    {.key = "hover", .label = "Hover", .type = UiFieldType::Color},
    {.key = "active", .label = "Active", .type = UiFieldType::Color},
    {.key = "text", .label = "Text", .type = UiFieldType::Color},
};

constexpr UiField kImageFields[] = {
    {.key = "path", .label = "Path", .type = UiFieldType::String},
    {.key = "width",
     .label = "Width",
     .type = UiFieldType::Number,
     .integer = true,
     .numberDefault = 128},
    {.key = "height",
     .label = "Height",
     .type = UiFieldType::Number,
     .integer = true,
     .numberDefault = 128},
};

constexpr UiField kConfirmFields[] = {
    {.key = "title", .label = "Title", .type = UiFieldType::String, .stringDefault = "Confirm"},
    {.key = "message",
     .label = "Message",
     .type = UiFieldType::String,
     .stringDefault = "Are you sure?"},
    {.key = "confirmLabel",
     .label = "Confirm label",
     .type = UiFieldType::String,
     .stringDefault = "Confirm"},
    {.key = "cancelLabel",
     .label = "Cancel label",
     .type = UiFieldType::String,
     .stringDefault = "Cancel"},
};

constexpr UiField kMenuFields[] = {
    {.key = "title", .label = "Title", .type = UiFieldType::String, .stringDefault = "GAME NAME"},
    {.key = "message", .label = "Message", .type = UiFieldType::String},
};

constexpr UiField kSettingsFields[] = {
    {.key = "title", .label = "Title", .type = UiFieldType::String, .stringDefault = "SETTINGS"},
};

template <std::size_t N> constexpr int CountFields(const UiField (&)[N]) {
    return static_cast<int>(N);
}

constexpr UiTemplateInfo kTemplates[] = {
    {"button", "Button", "Widgets", kButtonFields, CountFields(kButtonFields)},
    {"checkbox", "Checkbox", "Widgets", kCheckboxFields, CountFields(kCheckboxFields)},
    {"slider", "Slider", "Widgets", kSliderFields, CountFields(kSliderFields)},
    {"slider_int", "Slider int", "Widgets", kSliderIntFields, CountFields(kSliderIntFields)},
    {"input", "Input", "Widgets", kInputFields, CountFields(kInputFields)},
    {"combo", "Combo", "Widgets", kComboFields, CountFields(kComboFields)},
    {"radio_group", "Radio group", "Widgets", kRadioFields, CountFields(kRadioFields)},
    {"progress", "Progress", "Widgets", kProgressFields, CountFields(kProgressFields)},
    {"image", "Image", "Widgets", kImageFields, CountFields(kImageFields)},
    {"confirm", "Confirm", "Screens", kConfirmFields, CountFields(kConfirmFields)},
    {"menu", "Menu", "Screens", kMenuFields, CountFields(kMenuFields)},
    {"settings", "Settings", "Screens", kSettingsFields, CountFields(kSettingsFields)},
};

UiProp MakeProp(const UiField& field) {
    UiProp prop;
    int index = 0;

    prop.key = field.key;
    if (field.type == UiFieldType::Bool) {
        prop.kind = UiValueKind::Bool;
        prop.boolValue = field.boolDefault;
        return prop;
    }
    if (field.type == UiFieldType::Number) {
        prop.kind = UiValueKind::Number;
        prop.numberValue = field.numberDefault;
        return prop;
    }
    if (field.type == UiFieldType::StringList) {
        prop.kind = UiValueKind::StringList;
        for (index = 0; index < field.listCount; ++index) {
            prop.stringList.emplace_back(field.listDefault[index]);
        }
        return prop;
    }
    if (field.type == UiFieldType::Color) {
        prop.kind = UiValueKind::Color;
        prop.colorSet = false;
        prop.colorValue = {1.0f, 1.0f, 1.0f, 1.0f};
        return prop;
    }
    prop.kind = UiValueKind::String;
    if (field.stringDefault != nullptr) {
        prop.stringValue = field.stringDefault;
    }
    return prop;
}

// Alive screen entities, in entity order.
std::vector<Entity> ScreenEntities(Registry& registry) {
    std::vector<Entity> screens;
    std::size_t index = 0;

    if (!registry.IsComponentRegistered<UiComponent>()) {
        return screens;
    }
    for (index = 0; index < registry.GetEntityCount(); ++index) {
        if (IsUiScreenEntity(registry, Entity{index})) {
            screens.push_back(Entity{index});
        }
    }
    return screens;
}

// "Button(1)" shares the base "Button", so the next copy is "Button(2)".
std::string EntityNameBase(const std::string& name) {
    std::size_t open = 0;
    std::size_t index = 0;

    if (name.size() < 3 || name.back() != ')') {
        return name;
    }
    open = name.rfind('(');
    if (open == std::string::npos || open == 0 || open + 1 >= name.size() - 1) {
        return name;
    }
    for (index = open + 1; index + 1 < name.size(); ++index) {
        if (!std::isdigit(static_cast<unsigned char>(name[index]))) {
            return name;
        }
    }
    return name.substr(0, open);
}

bool EntityNameTaken(Registry& registry, const std::string& name, Entity skip) {
    std::size_t index = 0;
    Entity entity = 0;

    if (!registry.IsComponentRegistered<Name>()) {
        return false;
    }
    for (index = 0; index < registry.GetEntityCount(); ++index) {
        entity = Entity{index};
        if (entity == skip || !registry.IsEntityAlive(entity) ||
            !registry.HasComponent<Name>(entity)) {
            continue;
        }
        if (registry.RequireComponent<Name>(entity).value == name) {
            return true;
        }
    }
    return false;
}

std::string UniqueUiEntityName(Registry& registry, const std::string& base, Entity skip) {
    std::string name = base;
    std::string stem;
    std::string candidate;
    int suffix = 1;

    if (name.empty()) {
        name = "Entity";
    }
    if (!EntityNameTaken(registry, name, skip)) {
        return name;
    }
    stem = EntityNameBase(name);
    candidate = stem + "(1)";
    while (suffix < 10000 && EntityNameTaken(registry, candidate, skip)) {
        ++suffix;
        candidate = stem + "(" + std::to_string(suffix) + ")";
    }
    return candidate;
}

void AssignUniqueName(Registry& registry, Entity entity, const std::string& base) {
    if (!registry.IsComponentRegistered<Name>()) {
        return;
    }
    if (registry.HasComponent<Name>(entity)) {
        return;
    }
    registry.AddComponent<Name>(entity, Name::Create(UniqueUiEntityName(registry, base, entity)));
}

} // namespace

std::string UniqueUiPageName(Registry& registry, const std::string& base) {
    std::vector<std::string> pages = ListUiPages(registry);
    std::string name = base;
    std::string candidate;
    int suffix = 2;

    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char letter) { return static_cast<char>(std::tolower(letter)); });
    if (name.empty()) {
        name = "page";
    }
    candidate = name;
    while (std::find(pages.begin(), pages.end(), candidate) != pages.end()) {
        candidate = name + std::to_string(suffix);
        ++suffix;
    }
    return candidate;
}

const UiTemplateInfo* UiTemplates() {
    return kTemplates;
}

int UiTemplateCount() {
    return static_cast<int>(sizeof(kTemplates) / sizeof(kTemplates[0]));
}

const UiTemplateInfo* FindUiTemplate(const std::string& id) {
    int index = 0;

    for (index = 0; index < UiTemplateCount(); ++index) {
        if (id == kTemplates[index].id) {
            return &kTemplates[index];
        }
    }
    return nullptr;
}

bool IsUiWidget(const UiTemplateInfo& info) {
    return std::string(info.group) == "Widgets";
}

bool IsUiScreen(const UiTemplateInfo& info) {
    return !IsUiWidget(info);
}

std::optional<std::size_t> FindUiPropIndex(const std::vector<UiProp>& props, const std::string& key) {
    std::size_t index = 0;

    for (index = 0; index < props.size(); ++index) {
        if (props[index].key == key) {
            return index;
        }
    }
    return std::nullopt;
}

UiProp& EnsureUiProp(std::vector<UiProp>& props, const UiField& field) {
    std::optional<std::size_t> found = FindUiPropIndex(props, field.key);

    if (!found.has_value()) {
        props.push_back(MakeProp(field));
        return props.back();
    }
    return props[found.value()];
}

void FillUiProps(std::vector<UiProp>& props, const UiTemplateInfo& info) {
    int index = 0;

    props.clear();
    for (index = 0; index < info.fieldCount; ++index) {
        props.push_back(MakeProp(info.fields[index]));
    }
}

void ApplyUiTemplate(UiComponent& component, const std::string& id) {
    const UiTemplateInfo* info = FindUiTemplate(id);

    if (info == nullptr) {
        return;
    }
    component.scriptPath.clear();
    component.templateName = info->id;
    component.loaded = false;
    component.scriptTable = sol::lua_nil;
    FillUiProps(component.props, *info);
}

UiComponent MakeUiTemplate(const std::string& id) {
    UiComponent component;

    ApplyUiTemplate(component, id);
    return component;
}

void AttachUiTemplate(Registry& registry, Entity entity, const std::string& id) {
    UiComponent component = MakeUiTemplate(id);
    const UiTemplateInfo* info = FindUiTemplate(id);
    std::string base;

    if (component.templateName.empty() || info == nullptr) {
        return;
    }
    if (registry.IsComponentRegistered<UiComponent>() &&
        registry.HasComponent<UiComponent>(entity)) {
        // A replaced child keeps its place among the screen's children.
        component.order = registry.RequireComponent<UiComponent>(entity).order;
        registry.RemoveComponent<UiComponent>(entity);
    }
    if (IsUiScreen(*info)) {
        base = info->label;
        if (registry.IsComponentRegistered<Name>() && registry.HasComponent<Name>(entity) &&
            !registry.RequireComponent<Name>(entity).value.empty()) {
            base = registry.RequireComponent<Name>(entity).value;
        }
        component.page = UniqueUiPageName(registry, base);
    }
    UiPlugin::AttachToEntity(registry, entity, component);
    AssignUniqueName(registry, entity, info->label);
}

bool IsUiScreenEntity(Registry& registry, Entity entity) {
    const UiTemplateInfo* info = nullptr;

    if (!registry.IsComponentRegistered<UiComponent>() || !registry.IsEntityAlive(entity) ||
        !registry.HasComponent<UiComponent>(entity)) {
        return false;
    }
    info = FindUiTemplate(registry.RequireComponent<UiComponent>(entity).templateName);
    return info != nullptr && IsUiScreen(*info);
}

std::optional<Entity> FindUiScreenParent(Registry& registry, Entity entity) {
    Entity parent = 0;

    if (!registry.IsComponentRegistered<Parent>() || !registry.HasComponent<Parent>(entity)) {
        return std::nullopt;
    }
    parent = registry.RequireComponent<Parent>(entity).parent;
    if (!IsUiScreenEntity(registry, parent)) {
        return std::nullopt;
    }
    return parent;
}

std::vector<Entity> FindUiScreenChildren(Registry& registry, Entity screen) {
    std::vector<Entity> children;

    if (!registry.IsComponentRegistered<Parent>() ||
        !registry.IsComponentRegistered<UiComponent>()) {
        return children;
    }
    for (const Entity child : HierarchySystem::FindDirectChildren(registry, screen)) {
        if (registry.IsEntityAlive(child) && registry.HasComponent<UiComponent>(child)) {
            children.push_back(child);
        }
    }
    // FindDirectChildren walks a hash map, so its order means nothing.
    std::sort(children.begin(), children.end(), [&registry](Entity left, Entity right) {
        const int leftOrder = registry.RequireComponent<UiComponent>(left).order;
        const int rightOrder = registry.RequireComponent<UiComponent>(right).order;
        if (leftOrder != rightOrder) {
            return leftOrder < rightOrder;
        }
        return left < right;
    });
    return children;
}

std::vector<std::string> ListUiPages(Registry& registry) {
    std::vector<std::string> pages;

    for (const Entity screen : ScreenEntities(registry)) {
        const std::string& page = registry.RequireComponent<UiComponent>(screen).page;
        if (!page.empty() && std::find(pages.begin(), pages.end(), page) == pages.end()) {
            pages.push_back(page);
        }
    }
    return pages;
}

std::optional<Entity> FindUiPageScreen(Registry& registry, const std::string& page) {
    if (page.empty()) {
        return std::nullopt;
    }
    for (const Entity screen : ScreenEntities(registry)) {
        if (registry.RequireComponent<UiComponent>(screen).page == page) {
            return screen;
        }
    }
    return std::nullopt;
}

std::string UiPageOf(Registry& registry, Entity entity) {
    std::optional<Entity> screen;

    if (!registry.IsComponentRegistered<UiComponent>() ||
        !registry.HasComponent<UiComponent>(entity)) {
        return "";
    }
    if (IsUiScreenEntity(registry, entity)) {
        return registry.RequireComponent<UiComponent>(entity).page;
    }
    screen = FindUiScreenParent(registry, entity);
    if (screen.has_value()) {
        return registry.RequireComponent<UiComponent>(screen.value()).page;
    }
    return registry.RequireComponent<UiComponent>(entity).showOn;
}

std::string StartUiPage(Registry& registry) {
    std::vector<std::string> pages;

    for (const Entity screen : ScreenEntities(registry)) {
        const UiComponent& ui = registry.RequireComponent<UiComponent>(screen);
        if (ui.startPage && !ui.page.empty()) {
            return ui.page;
        }
    }
    pages = ListUiPages(registry);
    if (pages.empty()) {
        return "";
    }
    return pages.front();
}

void SetUiStartPage(Registry& registry, Entity screen) {
    for (const Entity other : ScreenEntities(registry)) {
        registry.RequireComponent<UiComponent>(other).startPage = other == screen;
    }
}

void RenameUiPage(Registry& registry, const std::string& from, const std::string& to) {
    std::shared_ptr<ComponentArray<UiComponent>> components;

    if (from.empty() || from == to || !registry.IsComponentRegistered<UiComponent>()) {
        return;
    }
    components = registry.GetComponentArray<UiComponent>();
    for (auto& [entity, ui] : *components) {
        if (ui.page == from) {
            ui.page = to;
        }
        if (ui.showOn == from) {
            ui.showOn = to;
        }
        if (ui.onClick.type == UiActionType::GoToPage && ui.onClick.target == from) {
            ui.onClick.target = to;
        }
        if (ui.onCancel.type == UiActionType::GoToPage && ui.onCancel.target == from) {
            ui.onCancel.target = to;
        }
    }
    if (!registry.HasResource<UiNavigation>()) {
        return;
    }
    UiNavigation& navigation = registry.GetResource<UiNavigation>();
    if (navigation.current == from) {
        navigation.current = to;
    }
    std::replace(navigation.history.begin(), navigation.history.end(), from, to);
}

Entity AddUiChild(Registry& registry, Entity screen, const std::string& id) {
    std::vector<Entity> siblings = FindUiScreenChildren(registry, screen);
    Entity child = registry.CreateEntity();
    int order = 0;

    if (registry.HasComponent<Position>(screen)) {
        registry.RequireComponent<Position>(child) = registry.RequireComponent<Position>(screen);
    }
    registry.RegisterComponent<Parent>();
    registry.AddComponent<Parent>(child, Parent::Create(screen));
    AttachUiTemplate(registry, child, id);
    if (!siblings.empty()) {
        order = registry.RequireComponent<UiComponent>(siblings.back()).order + 1;
    }
    if (registry.IsComponentRegistered<UiComponent>() &&
        registry.HasComponent<UiComponent>(child)) {
        registry.RequireComponent<UiComponent>(child).order = order;
    }
    return child;
}

void MoveUiChild(Registry& registry, Entity child, int direction) {
    std::optional<Entity> screen = FindUiScreenParent(registry, child);
    std::vector<Entity> siblings;
    std::vector<Entity>::iterator found;
    std::size_t index = 0;
    std::size_t from = 0;
    std::size_t to = 0;

    if (!screen.has_value() || direction == 0) {
        return;
    }
    siblings = FindUiScreenChildren(registry, screen.value());
    found = std::find(siblings.begin(), siblings.end(), child);
    if (found == siblings.end()) {
        return;
    }
    from = static_cast<std::size_t>(found - siblings.begin());
    if (direction < 0 && from == 0) {
        return;
    }
    if (direction > 0 && from + 1 >= siblings.size()) {
        return;
    }
    to = direction < 0 ? from - 1 : from + 1;
    // Equal orders would swap to the same values, so the siblings are renumbered first.
    for (index = 0; index < siblings.size(); ++index) {
        registry.RequireComponent<UiComponent>(siblings[index]).order = static_cast<int>(index);
    }
    registry.RequireComponent<UiComponent>(siblings[from]).order = static_cast<int>(to);
    registry.RequireComponent<UiComponent>(siblings[to]).order = static_cast<int>(from);
}

const char* UiActionLabel(UiActionType type) {
    const int index = static_cast<int>(type);

    if (index < 0 || index >= kUiActionTypeCount) {
        return kActionLabels[0];
    }
    return kActionLabels[index];
}

} // namespace Engine
