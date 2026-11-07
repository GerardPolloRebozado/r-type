#ifndef UI_UICOMPONENT_HPP_
#define UI_UICOMPONENT_HPP_

#include <array>
#include <sol/sol.hpp>
#include <string>
#include <vector>

namespace Engine {

/**
 * @brief Which slot of a UiProp holds the saved value.
 *
 * A future tool writes these names in the scene file: bool, number, string, stringList, color.
 */
enum class UiValueKind { Bool, Number, String, StringList, Color };

/**
 * @brief One saved field of a premade template.
 */
struct UiProp {
    std::string key;
    UiValueKind kind = UiValueKind::String;
    bool boolValue = false;
    double numberValue = 0.0;
    std::string stringValue;
    std::vector<std::string> stringList;
    /** @brief Used only when kind is Color. colorSet false keeps the template style. */
    bool colorSet = false;
    std::array<float, 4> colorValue{1.0f, 1.0f, 1.0f, 1.0f};
};

/**
 * @brief What a button does when it is clicked.
 *
 * Saved as: none, goToPage, back, callMethod, loadScene, quit.
 */
enum class UiActionType { None, GoToPage, Back, CallMethod, LoadScene, Quit };

/**
 * @brief One click action of a button or of a Confirm choice.
 */
struct UiAction {
    UiActionType type = UiActionType::None;
    /** @brief Page name, method name, or scene name depending on type. */
    std::string target;
};

/**
 * @brief Attaches a Lua UI script, or a premade template, to an entity.
 *
 * An empty templateName means scriptPath is a custom script. That script returns
 * a table with an optional onOpen, a draw(entity) called every frame inside the
 * ImGui pass, and an optional onClose. loaded and scriptTable are runtime only
 * and are not saved. A template stores its editable values in props. A screen's
 * widgets are child entities (Parent) drawn in order.
 */
struct UiComponent {
    /** @brief Path to the Lua script, relative to the project folder. */
    std::string scriptPath;

    /** @brief Stored with the scene. The runtime reloads when the path changes. */
    bool autoReload = true;

    /** @brief Premade template id. Empty means scriptPath is used instead. */
    std::string templateName;

    /** @brief Editable values for the template. */
    std::vector<UiProp> props;

    /** @brief Screens only: this screen's page name. */
    std::string page;

    /** @brief Screens only: shown first when the game starts. One screen at most. */
    bool startPage = false;

    /** @brief Standalone widgets only: page they appear on. Empty means always. */
    std::string showOn;

    /** @brief Draw order among the children of a screen (ascending, then entity id). */
    int order = 0;

    /** @brief Buttons, and Confirm's confirm button. */
    UiAction onClick;

    /** @brief Confirm's cancel button. */
    UiAction onCancel;

    /** @brief True after the script table has been loaded. */
    bool loaded = false;

    /** @brief Table returned by the script. */
    sol::table scriptTable = sol::lua_nil;

    UiComponent() = default;
    explicit UiComponent(const std::string& path, bool autoReload_ = true)
        : scriptPath(path), autoReload(autoReload_) {}
};

} // namespace Engine

#endif // UI_UICOMPONENT_HPP_
