#ifndef UI_UICOMPONENT_HPP_
#define UI_UICOMPONENT_HPP_

#include <sol/sol.hpp>
#include <string>

namespace Engine {

/**
 * @brief Attaches a Lua UI script to an entity.
 *
 * The script returns a table with an optional onOpen, a draw(entity) called
 * every frame inside the ImGui pass, and an optional onClose.
 * loaded and scriptTable are runtime only and are not saved.
 */
struct UiComponent {
    /** @brief Path to the Lua script, relative to the project folder. */
    std::string scriptPath;

    /** @brief Stored with the scene. The runtime reloads when the path changes. */
    bool autoReload = true;

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
