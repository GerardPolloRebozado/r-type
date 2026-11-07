#ifndef UI_UICATALOG_HPP_
#define UI_UICATALOG_HPP_

#include "ECS/Registry.hpp"
#include "UI/UiComponent.hpp"
#include <optional>
#include <string>
#include <vector>

namespace Engine {

/**
 * @brief Kind of value a catalog field stores on a UiComponent.
 */
enum class UiFieldType { String, Bool, Number, Enum, StringList, Color };

/**
 * @brief One editable field of a premade template.
 *
 * Defaults live here. A saved component stores only the current values.
 */
struct UiField {
    const char* key = "";
    const char* label = "";
    UiFieldType type = UiFieldType::String;
    /** @brief Number fields that are whole numbers, such as width or an index. */
    bool integer = false;
    bool boolDefault = false;
    double numberDefault = 0.0;
    const char* stringDefault = "";
    const char* const* options = nullptr;
    int optionCount = 0;
    const char* const* listDefault = nullptr;
    int listCount = 0;
};

/**
 * @brief A premade template the editor can place and the runtime can draw.
 */
struct UiTemplateInfo {
    const char* id = "";
    const char* label = "";
    /** @brief "Widgets" or "Screens". */
    const char* group = "";
    const UiField* fields = nullptr;
    int fieldCount = 0;
};

/** @brief Every premade template, widgets first, then screens. */
const UiTemplateInfo* UiTemplates();
int UiTemplateCount();

const UiTemplateInfo* FindUiTemplate(const std::string& id);

/** @brief True for a single widget. Screens are menu, settings, and confirm. */
bool IsUiWidget(const UiTemplateInfo& info);

/** @brief True for menu, settings, and confirm. */
bool IsUiScreen(const UiTemplateInfo& info);

std::optional<std::size_t> FindUiPropIndex(const std::vector<UiProp>& props,
                                           const std::string& key);

/** @brief Appends the field when the component does not have it yet. */
UiProp& EnsureUiProp(std::vector<UiProp>& props, const UiField& field);

void FillUiProps(std::vector<UiProp>& props, const UiTemplateInfo& info);

/**
 * @brief Replaces the template and props. A custom script path is cleared.
 */
void ApplyUiTemplate(UiComponent& component, const std::string& id);

/** @brief A component already filled with that template's defaults. */
UiComponent MakeUiTemplate(const std::string& id);

/**
 * @brief Unique lowercase page name derived from base (entity Name or template label).
 *
 * Empty base becomes "page". Existing names get a numeric suffix: "menu", "menu2".
 */
std::string UniqueUiPageName(Registry& registry, const std::string& base);

/**
 * @brief Attaches that template, replacing a UI component already on the entity.
 *
 * A screen with an empty page gets a unique lowercase page name derived from
 * the entity Name, or from the template label, e.g. "menu", "menu2".
 */
void AttachUiTemplate(Registry& registry, Entity entity, const std::string& id);

/** @brief True if the entity has a UiComponent whose template is a screen. */
bool IsUiScreenEntity(Registry& registry, Entity entity);

/** @brief Screen parent of entity (Parent component -> screen UI), if any. */
std::optional<Entity> FindUiScreenParent(Registry& registry, Entity entity);

/** @brief Direct children of a screen that carry a UiComponent, sorted by (order, id). */
std::vector<Entity> FindUiScreenChildren(Registry& registry, Entity screen);

/** @brief Page names of every screen, entity order, no duplicates. */
std::vector<std::string> ListUiPages(Registry& registry);

/** @brief Screen whose page == name. */
std::optional<Entity> FindUiPageScreen(Registry& registry, const std::string& page);

/** @brief Screen: its page. Child of a screen: the parent's page. Otherwise showOn. */
std::string UiPageOf(Registry& registry, Entity entity);

/** @brief startPage screen, else first screen, else "". */
std::string StartUiPage(Registry& registry);

/** @brief Sets startPage on this screen and clears it on every other one. */
void SetUiStartPage(Registry& registry, Entity screen);

/** @brief Renames a page and updates every showOn and GoToPage target that used it. */
void RenameUiPage(Registry& registry, const std::string& from, const std::string& to);

/**
 * @brief New child entity of a screen with that widget template.
 *
 * The child gets Position (copy of the screen's), Parent, Name(label), and
 * order = last + 1.
 * @return The new entity.
 */
Entity AddUiChild(Registry& registry, Entity screen, const std::string& id);

/** @brief Swaps order with the previous (-1) or next (+1) sibling. */
void MoveUiChild(Registry& registry, Entity child, int direction);

/** @brief Combo labels: "None","Go to page","Back","Call method","Load scene","Quit". */
const char* UiActionLabel(UiActionType type);

inline constexpr int kUiActionTypeCount = 6;

} // namespace Engine

#endif // UI_UICATALOG_HPP_
